<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class DhcpSectionCreationDialog extends GenericDialog
{
  protected $post_cancel = 'cancel_section';
  protected $post_finish = 'create_section';
  protected $classtype;
  protected $parentdn;
  protected $plugin;
  protected $initialObjectValue;
  protected $selectAttribute;

  function __construct($simplePlugin, &$attribute, $classtype, $parentdn, $editingValue = NULL)
  {
    $this->plugin     = $simplePlugin;
    $this->attribute  = $attribute;
    $this->classtype  = $classtype;
    $this->parentdn   = $parentdn;
    if ($editingValue === NULL) {
      $this->dialog = FALSE;
    } else {
      $this->dialog = new $classtype($this->plugin, $editingValue['dn'], $editingValue);
    }
    $this->initialObjectValue = $editingValue;

    $sections = dhcpConfiguration::$sectionMap[$this->classtype];
    $t_sections = array();
    foreach ($sections as $section) {
      $t_sections[$section] = $this->plugin->types[$section];
    }
    asort($t_sections);
    $this->selectAttribute = new SelectAttribute(
      _('Section'), _('Choose section type to create'),
      'dhcpSectionType', TRUE,
      array_keys($t_sections), '', array_values($t_sections)
    );
  }

  function execute ()
  {
    if ($this->dialog !== FALSE) {
      if (isset($_POST['dhcp_cancel'])) {
        return $this->handle_cancel();
      }
      if (isset($_POST['dhcp_save'])) {
        return $this->save_dhcp();
      }
    } else {
      $this->selectAttribute->loadPostValue();
      $this->selectAttribute->applyPostValue();
      if (isset($_POST[$this->post_cancel])) {
        return $this->handle_cancel();
      }
      if (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) {
        return $this->handle_finish();
      }
    }
    return $this->dialog_execute();
  }

  function dialog_execute ()
  {
    if ($this->dialog !== FALSE) {
      $this->dialog->save_object();
      return $this->dialog->execute();
    }
    $smarty   = get_smarty();

    $smarty->assign('section',        _('Create new DHCP section'));
    $smarty->assign('sectionId',      'dhcpCreation');
    $smarty->assign('sectionClasses', '');
    $attributes = array();
    $smarty->assign($this->selectAttribute->getAcl().'ACL', 'rwcdm');
    $this->selectAttribute->renderAttribute($attributes, FALSE);
    $smarty->assign('attributes', $attributes);
    $sections = array(
      'dhcp' => $smarty->fetch(get_template_path('simpleplugin_section.tpl'))
    );
    $smarty->assign('sections', $sections);
    $smarty->assign('hiddenPostedInput', get_class($this).'_posted');
    $smarty->assign('focusedField', $this->selectAttribute->getLdapName());

    return $smarty->fetch(get_template_path('simpleplugin.tpl'))."\n".
    '<p class="plugbottom">'."\n".
    '  <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
    '  <input type="submit" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'."\n".
    '</p>';
  }

  function handle_finish ()
  {
    $section = $this->selectAttribute->getValue();

    if (isset(dhcpConfiguration::$sectionMap[$section])) {
      $this->dialog   = new $section($this->plugin, 'new', $this->parentdn);
      $this->parentdn = FALSE;
      return $this->dialog->execute();
    } else {
      return FALSE;
    }
  }

  function handle_cancel ()
  {
    if ($this->initialObjectValue !== NULL) {
      $this->attribute->editEnded($this->parentdn, $this->initialObjectValue);
    }
    return FALSE;
  }

  function save_dhcp()
  {
    $this->dialog->save_object();
    $messages = $this->dialog->check();
    if (count($messages)) {
      msg_dialog::displayChecks($messages);
      return $this->dialog_execute();
    }

    $dn     = $this->dialog->dn;
    $data   = $this->dialog->save();
    $this->attribute->editEnded($data['dn'], $data, ($this->parentdn === FALSE ? FALSE : $this->parentdn));
    return FALSE;
  }
}
?>
