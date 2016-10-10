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

class TemplateFileDialog extends GenericSimplePluginDialog
{
  protected $dialogClass = 'faiTemplateEntry';
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
    global $config;
    if (isset($attrs['dn'])) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($attrs['dn']);
      $ldap->search($this->templateFilter, array('*'), 'one');
      $this->value = array();
      while ($subattrs = $ldap->fetch()) {
        $attrsWrapper = new stdClass();
        $attrsWrapper->attrs = $subattrs;
        $dialog = new faiTemplateEntry($subattrs['dn'], $attrsWrapper);
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
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->cd($this->plugin->dn);
    $ldap->search($this->templateFilter, array('dn'), 'one');
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

  protected function getAttributeArrayValue($key, $value)
  {
    return array(
      $value['cn'],
      $value['description'],
      humanReadableSize(strlen($value['FAItemplateFile'])),
    );
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      $used_cn = array();
      foreach ($this->value as $value) {
        if (isset($used_cn[$value['cn']])) {
          return sprintf(_('There are several files with the same path: %s'), $value['cn']);
        }
        $used_cn[$value['cn']] = TRUE;
      }
    }
  }
}

class faiTemplate extends faiSimplePluginClass
{
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

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
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

  function ldap_save ()
  {
    global $config;
    $errors = parent::ldap_save();
    $this->attributesAccess['templateFiles']->postLdapSave($config->get_ldap_link());
    return $errors;
  }
}
?>
