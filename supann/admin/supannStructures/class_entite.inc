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

class entite extends plugin
{
  /* entite attributes */
  var $ou                       = "";
  var $description              = "";
  var $postalAddress            = "";
  var $telephoneNumber          = "";
  var $facsimileTelephoneNumber = "";
  var $supannTypeEntite         = "";
  var $supannCodeEntite         = "";
  var $supannCodeEntiteParent   = "";
  var $supannRefId              = "";


  var $attributes = array("ou", "description", "postalAddress", "telephoneNumber",
                          "facsimileTelephoneNumber", "supannTypeEntite",
                          "supannCodeEntite", "supannCodeEntiteParent","supannRefId");

  /* Do not append the structural object classes here, they are added dynamically in the constructor */
  var $objectclasses = array("top", "supannEntite","organizationalUnit");

  /* attributes that are both required and unique */
  var $required_attributes = array(
                                    "ou",
                                    "supannCodeEntite",
                                  );

  var $availableEntity;
  var $entiteLabels;

  function entite (&$config, $dn = NULL)
  {
    plugin::plugin ($config, $dn);

    foreach ($this->attributes as $attr) {
      if (isset($this->attrs[$attr])) {
        $this->$attr = $this->attrs[$attr][0];
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    $ldap->search("(objectClass=supannEntite)",array("supannCodeEntite","ou","o"));

    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        if (isset($attrs['ou'][0])) {
          $this->availableEntity[$attrs['supannCodeEntite'][0]] = $attrs['ou'][0]." (".$attrs['supannCodeEntite'][0].")";
        } else {
          $this->availableEntity[$attrs['supannCodeEntite'][0]] = $attrs['o'][0]." (".$attrs['supannCodeEntite'][0].")";
        }
      }
    }
    $this->availableEntity[""] = "";

       /* Generate Liste of entite */
    if (file_exists(SUPANN_DIR.'/entite')) {
      $entiteList = file(SUPANN_DIR.'/entite');
      foreach ($entiteList as $line) {
        if (!preg_match ("/^#/", $line)) {
          $entite_line=preg_split('/;/',trim($line));

          $this->entiteLabels[$entite_line[0]]  = $this->truncate_label($entite_line[1]);
        }
      }
    }
    $this->entiteLabels[""] = "";

    $this->orig_dn = $this->dn;

    $this->is_account = TRUE;
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();
    $smarty = get_smarty();
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $code=preg_match("/^{[^}]+}/",$this->supannTypeEntite);
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $code, "Supann code:");

    $smarty->assign("typesEntites", $this->entiteLabels);
    $smarty->assign("supannEntiteParentList",$this->availableEntity);

    return($smarty->fetch (get_template_path('entite.tpl', TRUE)));
  }

  /* Save to LDAP */
  function save()
  {
    plugin::save();

    $ret = 0;
    $ldap = $this->config->get_ldap_link();

    if ($this->dn == 'new') {
      $this->dn = $this->compute_dn();
    }

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
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
      $ret = 1;
    }

    /* Post that we've done*/
    $this->handle_post_events($mode);

    return $ret;
  }

  /* Save data to object */
  function save_object()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->ou, "ou");
    /* Save additional values for possible next step */
    if (isset($_POST['entite_posted'])) {
      plugin::save_object();
    }
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

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

  function remove_from_parent()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "dn");
    plugin::remove_from_parent();

    $ldap = $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }

    new log("remove","entite/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Remove ACL dependencies, too */
    acl::remove_acl_for($this->dn);

    /* Send signal to the world that we've done */
    $this->handle_post_events("remove");
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return "supannCodeEntite=".$this->supannCodeEntite.",".get_ou('supannStructuresRDN').$this->config->current['BASE'];
  }

  /* Not used anymore as code is left to use choice.
   * But this function might be used if one day we want to add code suggestions */
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

  /* return the 64 first chars and "…" after if text is longer */
  function truncate_label($str,$len = 64)
  {
    if (strlen($str) > $len) {
      return substr($str,0,$len)."…";
    } else {
      return $str;
    }
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Generic"),
      "plDescription" => _("SUPANN Entite Settings"),
      "plSelfModify"  => FALSE,
      "plSection"     => array("administration"),
      "plCategory"    => array("supann"),
      "plObjectType"  => array("entite" => array("name" => _("SUPANN Entite"),
                                                "filter" => "(&(objectClass=supannEntite)(objectClass=organizationalUnit))")),

      "plProvidedAcls"    => array(
        "ou"                        =>  _("ou"),
        "description"               =>  _("Description"),
        "postalAddress"             =>  _("postalAddress"),
        "telephoneNumber"           =>  _("telephoneNumber"),
        "facsimileTelephoneNumber"  =>  _("facsimileTelephoneNumber"),

        "supannTypeEntite"        =>  _("supannTypeEntite"),
        "supannCodeEntite"        =>  _("supannCodeEntite"),
        "supannCodeEntiteParent"  =>  _("supannCodeEntiteParent"),
        "supannRefId"             =>  _("supannRefId"),
      )
    );
  }
}
?>
