<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class etablissement extends plugin
{
  var $o                        = "";
  var $telephoneNumber          = "";
  var $facsimileTelephoneNumber = "";
  var $postalAddress            = "";
  var $l                        = "";
  var $description              = "";
  var $eduOrgHomePageURI        = "";
  var $eduOrgLegalName          = "";
  var $eduOrgWhitePagesURI      = "";
  var $eduOrgSuperiorURI        = "";
  var $supannEtablissement      = "";

  var $supannCodeEntite         = "";

  var $orig_o                   = "";

  var $attributes     = array("o","telephoneNumber","facsimileTelephoneNumber",
                              "postalAddress","l","description",
                              "eduOrgHomePageURI","eduOrgLegalName","eduOrgWhitePagesURI",
                              "eduOrgSuperiorURI","supannEtablissement","supannCodeEntite");

  /* attributes that are both required and unique */
  var $required_attributes = array(
                                    "o",
                                    "supannEtablissement",
                                    "supannCodeEntite",
                                    "eduOrgLegalName"
                                  );

  var $objectclasses  = array("top", "supannEntite","organization","supannOrg","eduOrg");

  var $set_root = FALSE;

  function etablissement (&$config, $dn)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "dn:");
    plugin::plugin ($config, $dn);

    $this->orig_dn = $dn;
    $this->orig_o = $this->o;

    $this->is_account = TRUE;
  }

  function execute()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "", "");
    /* Call parent execute */
    plugin::execute();

    $smarty = get_smarty();

    foreach($this->attributes as $attribute) {
      $smarty->assign($attribute,$this->$attribute);
      $smarty->assign($attribute."ACL",$this->getacl($attribute));

    }

    $root_code = $this->root_code();
    $smarty->assign("root_available",$root_code === FALSE);
    $smarty->assign("is_root",($root_code !== FALSE)&&($root_code == $this->supannEtablissement));

    return $smarty->fetch(get_template_path('etablissement.tpl', TRUE));
  }

  function remove_from_parent()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "dn");
    plugin::remove_from_parent();

    $ldap = $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }

    new log("remove","etablissement/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Remove ACL dependencies, too */
    acl::remove_acl_for($this->dn);

    /* If we're the root etablissement, delete it too */
    $root_code = $this->root_code();
    if (($root_code !== FALSE) && ($root_code == $this->supannEtablissement)) {
      $dn = "o=".$this->o.",".$this->config->current['BASE'];
      $ldap->rmdir($dn);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_DEL, get_class()));
      }
      /* Remove ACL dependencies, too */
      acl::remove_acl_for($dn);
    }

    /* Send signal to the world that we've done */
    $this->handle_post_events("remove");
  }


  /* Save data to object */
  function save_object()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->o, "o");
    /* Save additional values for possible next step */
    if (isset($_POST['etablissement_posted'])) {
      $root_code = $this->root_code();
      if(isset($_POST['o']) && ($root_code !== FALSE) && ($root_code == $this->supannEtablissement)) {
        unset($_POST['o']);
      }

      plugin::save_object();

      if (isset($_POST['set_root'])) {
        $this->set_root = TRUE;
      }
    }
  }

  /* Save to LDAP */
  function save()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->o, "o");
    /* New accounts need proper 'dn', propagate it to remaining objects */
    if ($this->dn == 'new') {
      $this->dn = $this->compute_dn();
    }

    plugin::save();

    $ldap = $this->config->get_ldap_link();


    /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
       new entries. So do a check first... */
    $ldap->cat($this->dn, array('dn'));
    if ($ldap->fetch()) {
      $mode = "modify";
    } else {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->supannCodeEntite, "code");
      $mode = "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
        $ret = 1;
      }
    }

    /* Write back to ldap */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->$mode($this->attrs);

    if ($this->initially_was_account) {
      new log("modify","etablissement/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    } else {
      new log("create","etablissement/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    $ret = 0;
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
      $ret = 1;
    }

    $root_mode = "";
    $root_code = $this->root_code();
    if (($root_code === FALSE) && ($this->set_root)) {
      /* Set this etablissement as root one */
      $root_attrs = array();
      foreach ($this->attributes as $attribute) {
        $root_attrs[$attribute] = $this->$attribute;
      }
      $root_mode = "add";
    } else if (($root_code !== FALSE) && ($root_code == $this->supannEtablissement)) {
      /* We are the root etablissement, we need to update it */
      $root_attrs = $this->attrs;
      $root_mode = "modify";
    }

    if (!empty($root_mode)) {
      $root_attrs['objectClass'] = array("top", "dcObject","organization","supannOrg","eduOrg");
      $root_attrs['dc'] = "foobar";
      unset($root_attrs['supannCodeEntite']);
      $dn = "o=".$this->o.",".$this->config->current['BASE'];
      $ldap->cd($dn);
      $this->cleanup();
      $ldap->$root_mode($root_attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
        $ret = 1;
      }
    }

    /* Post that we've done*/
    $this->handle_post_events($mode);

    return $ret;
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message = plugin::check();

    /* Check for valid input */
    $root_code = $this->root_code();
    if ((($root_code !== FALSE) && ($root_code == $this->supannEtablissement)) ||
        (($root_code === FALSE) && ($this->set_root))) {
      if (!tests::is_uid($this->o)) {
        if (strict_uid_mode()) {
          $message[] = msgPool::invalid(_("Name"), $this->o, "/[a-z0-9_-]/");
        } else {
          $message[] = msgPool::invalid(_("Name"), $this->o, "/[a-z0-9_-]/i");
        }
      }
    }
    if (!tests::is_uid($this->supannCodeEntite)) {
      if (strict_uid_mode()) {
        $message[] = msgPool::invalid(_("Name"), $this->supannCodeEntite, "/[a-z0-9_-]/");
      } else {
        $message[] = msgPool::invalid(_("Name"), $this->supannCodeEntite, "/[a-z0-9_-]/i");
      }
    }

    /* Check that required attributes are there */
    foreach ($this->required_attributes as $attribute) {
      if ($this->acl_is_writeable($attribute)) {
        if ($this->$attribute == "")  {
          $message[] = msgPool::required(_($attribute));
        } else {
          /* check unicity */
          $ldap = $this->config->get_ldap_link();
          $ldap->cd(get_ou('supannStructuresRDN').$this->config->current['BASE']);
          $ldap->search("($attribute=".$this->$attribute.")");
          while ($attrs = $ldap->fetch()) {
            if ($attrs['dn'] != $this->orig_dn) {
              $message[] = msgPool::duplicated(_($attribute));
              break;
            }
          }
        }
      }
    }

    return $message;
  }

  function compute_dn()
  {
    return "supannCodeEntite=".$this->supannCodeEntite.",".get_ou('supannStructuresRDN').$this->config->current['BASE'];
  }

  function generate_code()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd(get_ou('supannStructuresRDN').$this->config->current['BASE']);
    $ldap->search("(objectClass=supannEntite)", array("supannCodeEntite"));
    $code = 0;
    while ($attr = $ldap->fetch()) {
      $code_ldap = intval($attr['supannCodeEntite'][0]);
      if ($code_ldap > $code) {
        $code = $code_ldap;
      }
    }
    $code++;

    return $code;
  }

  function root_code()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->config->current['BASE']);
    $ldap->ls("(objectClass=supannOrg)");

    if($ldap->count() > 1) {
      msg_dialog::display(_("LDAP error"), "There are several etablissements at root!");
      return FALSE;
    } else if ($ldap->count() > 0) {
      $attr = $ldap->fetch();
      $code = $attr['supannEtablissement'][0];
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $code, "code");
      return $code;
    } else {
      return FALSE;
    }
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
          "plShortName"     => _("supann Etablissement Settings"),
          "plDescription"   => _("supann Etablissement Settings"),
          "plSelfModify"    => FALSE,
          "plDepends"       => array(),
          "plPriority"      => 0,
          "plSection"       => array("administration"),
          "plCategory"      => array("supann"),

          "plProvidedAcls"  => array(
              "o"                         =>  _("o"),
              "telephoneNumber"           =>  _("telephoneNumber"),
              "facsimileTelephoneNumber"  =>  _("facsimileTelephoneNumber"),
              "postalAddress"             =>  _("postalAddress"),
              "l"                         =>  _("l"),
              "description"               =>  _("Description"),
              "eduOrgHomePageURI"         =>  _("eduOrgHomePageURI"),
              "eduOrgLegalName"           =>  _("eduOrgLegalName"),
              "eduOrgWhitePagesURI"       =>  _("eduOrgWhitePagesURI"),
              "eduOrgSuperiorURI"         =>  _("eduOrgSuperiorURI"),
              "supannEtablissement"       =>  _("supannEtablissement"),
              "supannCodeEntite"          =>  _("supannCodeEntite"),
            )
        );
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
  }

}
?>
