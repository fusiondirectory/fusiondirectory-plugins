<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class servRepositorySetup  extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;
  var $attributes       = array("Release","ParentServer","Url","cn");
  var $objectclasses    = array("whatever");

  /* Attributes */
  var $Release          = "";
  var $Archs            = array();
  var $ParentServer     = "";
  var $Url              = "";
  var $Sections         = array();
  var $Install          = "install";
  var $Local            = "local";
  var $initialy_was     = FALSE;
  var $cn               = "";
  var $parent           = "";

  function servRepositorySetup (&$config, $dn = NULL, $data = FALSE)
  {
    plugin::plugin ($config, $dn);
    if ($data != FALSE) {
      foreach(array("Sections","Release","Archs","Url","ParentServer","Install","Local","initialy_was") as $atr) {
        if (isset($data[$atr])) {
          $this->$atr = $data[$atr];
        }
      }
    }
  }

  function GetName()
  {
    return $this->Release;
  }

  function is_new_name()
  {
    if (!$this->initialy_was) {
      return TRUE;
    } else {
      if ($this->Release != $this->initialy_was) {
        return TRUE;
      }
    }
    return FALSE;
  }



  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty = get_smarty();

    if (preg_match("/w/", $this->parent->getacl("Section"))) {
      if ((isset($_POST['AddSection'])) && (isset($_POST['SectionName'])) && (!empty($_POST['SectionName']))) {

        /* Replace multiple spaces with a single, and cut of white spaces (trim)*/
        $val = preg_replace("/\ \ * /", " ", trim($_POST['SectionName']));

        /* check if there are more than one entry given ( "section1 section2 )*/
        if (preg_match("/ /", $val)) {

          /* Generate list of new section names */
          $vals = explode(" ", $val);

          /* Add new entries */
          foreach($vals as $entry) {
            $entry = trim($entry);
            if (!empty($entry)) {
              $this->Sections[$entry] = $entry;
            }
          }
        } else {
          $this->Sections[$val] = $val;
        }
      }

      foreach($_POST as $name => $value) {
        if (preg_match("/^delete_/", $name)) {

          $val = preg_replace("/^delete_/", "", $name);
          $val = base64_decode(preg_replace("/_.*$/", "", $val));

          if (isset($this->Sections[$val])) {
            unset($this->Sections[$val]);
          }
        }
      }
    }

    $divlist = new divSelectBox("servRepositorySetup");
    $divlist->setHeight("220");
    if (preg_match("/w/", $this->parent->getacl("Section"))) {
      $dellink = "<input type='image' src='images/lists/trash.png' title='delete' alt='delete' name='delete_%s' value='%s'>";
    } else {
      $dellink = "";
    }

    foreach($this->Sections as $sec) {
      $divlist->AddEntry(array(
              array("string" => $sec),
              array("string" => sprintf($dellink, base64_encode($sec), $sec),
                  "attach" => "style='border-right:0px;width:20px;'")
            ));
    }

    $smarty->assign("Sections", $divlist->DrawList());

    /* Get && assign acls */
    $tmp = $this->parent->plInfo();
    foreach ($tmp['plProvidedAcls'] as $name => $translated) {
      $smarty->assign($name."ACL", $this->parent->getacl($name));
    }

    /* Assign values */
    foreach($this->attributes as $attr) {
      $smarty->assign($attr, $this->$attr);
    }
    $smarty->assign("Install", $this->Install);
    $smarty->assign("Local", $this->Local);
    $smarty->assign("Archs", implode($this->Archs,','));

    $tmp = $this->getParentServers();
    $smarty->assign("ParentServers",    $tmp);
    $smarty->assign("ParentServerKeys", array_flip($tmp));

    return $smarty->fetch(get_template_path('servRepositorySetup.tpl', TRUE, dirname(__FILE__)));
  }

  /* Save data to object */
  function save_object()
  {
    if (isset($_POST['servRepositorySetup_Posted'])) {
      if (isset($_POST['Install'])) {
        $this->Install = "install";
      } else {
        $this->Install = "update";
      }
      if (isset($_POST['Local'])) {
        $this->Local = "local";
      } else {
        $this->Local = "network";
      }
      if (isset($_POST['Archs'])) {
        $this->Archs = preg_split("/[\s,;]+/", $_POST['Archs']);
      }
      foreach($this->attributes as $attr) {
        if (($this->parent->acl_is_writeable($attr)) && (isset($_POST[$attr]))) {
          $this->$attr = $_POST[$attr];
        }
      }
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message = plugin::check();

    if (empty($this->Release)) {
      $message[] = msgPool::required(_("Release"));
    }

    if (empty($this->Url)) {
      $message[] = msgPool::required(_("URL"));
    }

    return $message;
  }


  /* Save to LDAP */
  function save()
  {
    $tmp = array();
    $tmp['ParentServer']  = $this->ParentServer;
    $tmp['Url']           = $this->Url;
    $tmp['Release']       = $this->Release;
    $tmp['Archs']         = $this->Archs;
    $tmp['Sections']      = $this->Sections;
    $tmp['Install']       = $this->Install;
    $tmp['Local']         = $this->Local;
    return $tmp;
  }

  function getParentServers()
  {
    $ret  = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=FAIrepositoryServer)", array("*"));
    while ($attr = $ldap->fetch()) {
      if ($attr['cn'][0] == $this->cn) {
        continue;
      }
      $ret[$attr['cn'][0]] = $attr['cn'][0];
    }

    $ret = array_merge($ret, $this->GetHookElements());

    $ret['none'] = "&nbsp;";
    asort($ret);
    return($ret);
  }

  /* this funtions calls a defined hook
      and parses all additional serverdata
   */
  function GetHookElements()
  {
    $ret = array();
    $cmd = $this->config->search("servrepository", "repositoryBranchHook", array('tabs'));
    if (!empty($cmd)) {
      $res  = shell_exec($cmd);
      $res2 = trim($res);
      if (!$res || empty($res2)) {
        msg_dialog::display(_("Error"), msgPool::cmdexecfailed( "repositoryBranchHook",
                                                                $cmd,
                                                                _("Repository service")),
                                                                ERROR_DIALOG);
      } else {
        $tmp = preg_split("/\n/", $res);
        foreach($tmp as $hook) {
          /* skip empty */
          if(empty($hook)) continue;

          if(preg_match("/;/", $hook)) {
            $hookinfo = explode(";", $hook);
            $ret[$hookinfo[0]] = $hookinfo[0];
          } else {
            $ret[$hook] = $hook;
          }
        }
      }
    }
    return $ret;
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
