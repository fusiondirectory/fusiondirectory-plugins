<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class TemplateFileDialog extends GenericDialog
{
  protected $dialogClass = 'faiTemplateEntry';

  //~ protected $post_cancel = 'CancelDisk';
  //~ protected $post_finish = 'SaveDisk';

  protected $initialDialogValue = NULL;

  function __construct($simplePlugin, &$attribute, $value = array())
  {
    $this->attribute  = $attribute;
    $this->dialog = new $this->dialogClass($simplePlugin->config, $simplePlugin->dn);

    $base = $simplePlugin->dn;
    if ($base == 'new') {
      $base = $simplePlugin->base;
    }
    $this->dialog->set_acl_base($base);
    $this->dialog->set_acl_category('fai');
    if (!empty($value)) {
      $this->initialDialogValue = $value;
      foreach ($this->dialog->attributes as $attr) {
        if (isset($value[$attr])) {
          $this->dialog->$attr = $value[$attr];
        }
      }
    }
  }

  function dialog_execute ()
  {
    $buttons = "<div style='width:100%; text-align:right; clear:both; float:none;'>".
               "  <input type='submit' name='add_finish' value='".msgPool::saveButton()."'/>&nbsp;".
               "  <input type='submit' name='add_cancel' value='".msgPool::cancelButton()."'/>".
               "</div>";
    $this->dialog->save_object();
    return $this->dialog->execute().$buttons;
  }

  function handle_finish ()
  {
    $this->dialog->save_object();
    if (count($this->dialog->check())) {
      foreach ($this->dialog->check() as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
      return $this->dialog->execute();
    }

    $this->attribute->diskConfigured($this->dialog->save());
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}

class FAITemplateFilesAttribute extends DialogOrderedArrayAttribute
{
  protected $dialogClass = 'TemplateFileDialog';
  protected $templateAttributes = array(
    'cn','description',
    'FAItemplatePath','FAItemplateFile','FAIowner','FAImode'
  );

  protected $templateFilter = '(&(objectClass=FAIclass)(objectClass=FAItemplateEntry))';

  function __construct ($label, $description, $ldapName, $values = array(), $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, FALSE, $values, $acl);
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs['dn'])) {
      $ldap = $this->plugin->config->get_ldap_link();
      $ldap->ls($this->templateFilter, $attrs['dn']);
      $this->value = array();
      while ($subattrs = $ldap->fetch()) {
        $attrsWrapper = new stdClass();
        $attrsWrapper->attrs = $subattrs;
        $dialog = new faiTemplateEntry($simplePlugin->config, $simplePlugin->dn, $attrsWrapper);
        $value = array();
        foreach ($this->templateAttributes as $attribute) {
          $value[$attribute] = $dialog->$attribute;
        }
        $this->value[] = $value;
      }
    } else {
      $this->resetToDefault();
    }
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
  {
    /* Remove crap made by plugin */
    unset ($attrs[$this->getLdapName()]);
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->ls($this->templateFilter, $this->plugin->dn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir($dn);
    }
    /* Then add our values */
    foreach ($this->value as $val) {
      $attrs = $val;
      $attrs['objectClass'] = array('top','FAIClass','FAItemplateEntry');
      $sub_dn = $this->compute_sub_dn($val);
      $this->ldap_add($ldap, $sub_dn, $attrs);
    }
  }

  protected function handleEdit($key)
  {
    parent::handleEdit($key);
  }
  function compute_sub_dn ($value)
  {
    return 'cn='.$value['cn'].','.$this->plugin->dn;
  }

  protected function ldap_add ($ldap, $dn, $attrs)
  {
    unset($attrs['dn']);
    $ldap->cd($dn);
    foreach (array_keys($attrs) as $index) {
      if ((is_array($attrs[$index]) && (count($attrs[$index]) == 0)) || ($attrs[$index] == '')) {
        unset($attrs[$index]);
      }
    }
    $ldap->add($attrs);
    if (!$ldap->success()) {
      msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
    }
  }

  protected function getAttributeArrayValue($value)
  {
    return array(
      $value['cn'],
      $value['descrition'],
      $value['FAItemplatePath'],
      humanReadableSize(strlen($value['FAItemplateFile'])),
    );
  }
}

class faiTemplate extends faiSimplePluginClass
{
  var $mainTab = TRUE;

  var $objectclasses    = array('top','FAIclass','FAItemplate');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Template'),
      'plDescription' => _('FAI template'),
      'plObjectType'  => array(
        'faiTemplate' => array(
          'name'        => _('FAI template'),
          'filter'      => 'objectClass=FAItemplate',
          'aclCategory' => 'fai',
          'ou'          => get_ou('faiTemplateRDN'),
          'icon'        => 'geticon.php?context=applications&icon=fai-template&size=16'
        )
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Class name'), _('Template class name'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('Short description of the class'),
            'description', FALSE
          ),
        )
      ),
      'table' => array(
        'name'  => _('Template files'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new FAITemplateFilesAttribute (
            '', _('Template files in this class'), 'templateFiles'
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  function ldap_save ($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);
    $this->attributesAccess['templateFiles']->postLdapSave($this->config->get_ldap_link());
  }
}
?>
