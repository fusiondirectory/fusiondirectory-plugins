<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016 FusionDirectory

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


class ejbcaCertificateSelectManagement extends simpleSelectManagement
{
  protected $objectTypes = array('ejbcaCertificate');

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'userCertificate');
}

class EjbcaCertSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'ejbcaCertificateSelectManagement';
}

class EjbcaUserCertsAttribute extends GenericDialogAttribute
{
  protected $dialogClass      = 'EjbcaCertSelectDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, 'dn', NULL, $acl);
  }

  function addValue ($dn, $entry)
  {
    if (!in_array($dn, $this->value)) {
      $this->value[]    = $dn;
      $this->displays[] = $entry;
    }
  }

  protected function fillDisplayValue ($i)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $value = $this->value[$i];
    $ldap->cat($value);
    if (!$ldap->count()) {
      unset($this->value[$i]);
    } else {
      $attrs = $ldap->fetch();
      $this->displays[$i] = $attrs;
    }
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $id = $this->getHtmlId();
      foreach ($_POST as $name => $value) {
        if (preg_match('/^'.$id.'_del_([^_]+)(_[xy])?$/', $name, $m)) {
          $this->removeValue($m[1]);
          break;
        } elseif (preg_match('/^'.$id.'_get_([^_]+)(_[xy])?$/', $name, $m)) {
          session::set('binary', $this->displays[$m[1]]['userCertificate;binary'][0]);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->displays[$m[1]]['cn'][0].'.der');
          header('location: getbin.php');
          exit();
        }
      }
      if (isset($_POST['add'.$this->getLdapName().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  function renderOnlyFormInput ()
  {
    $id   = $this->getHtmlId();
    $div  = new divSelectBox($id);
    // Updates and get display values
    $displays = $this->getDisplayValues();
    foreach ($displays as $key => $display_item) {
      $fields = array(
        array('string'  => $display_item['cn'][0])
      );
      $img = '<input type="image" src="geticon.php?context=actions&icon=download&size=16" name="'.$id.'_get_'.$key.'" class="center"/>';
      $img .= '<input type="image" src="geticon.php?context=actions&icon=edit-delete&size=16" name="'.$id.'_del_'.$key.'" class="center"/>';
      $fields[] = array('html' => $img, 'attach' => 'style="border-right:0px;width:32px;"');
      $div->AddEntry($fields);
    }
    $smarty = get_smarty();
    $smarty->assign("div_$id", $div->DrawList());
    return '{$div_'.$id.'}'."\n";
  }

  public function htmlIds()
  {
    $id = $this->getHtmlId();
    $ids = array('add'.$id.'_dialog');
    $nb_values = count($this->value);
    for ($i = 0; $i < $nb_values; ++$i) {
      $ids[] = $id.'_del_'.$i;
    }
    return $ids;
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" name="add'.$id.'_dialog" id="add'.$id.'_dialog"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' value="{msgPool type=addButton}"/>';
    return $buttons;
  }
}

?>
