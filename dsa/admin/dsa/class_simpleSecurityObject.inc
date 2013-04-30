<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2012-2013 FusionDirectory
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Handle a password and its hash method
 */
class UserPasswordAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    $temp = passwordMethod::get_available_methods();

    /* Create password methods array */
    $pwd_methods = array();
    foreach ($temp['name'] as $id => $name) {
      if (!$temp[$id]['object']->need_password()) {
        continue;
      }
      $pwd_methods[$name] = $name;
      if (!empty($temp[$id]['desc'])) {
        $pwd_methods[$name] .= " (".$temp[$id]['desc'].")";
      }
    }

    parent::__construct (
      $description, $ldapName,
      array(
        new SelectAttribute(
          _('Password method'), _('Password hash method to use'),
          $ldapName.'_pwstorage', TRUE,
          array_keys($pwd_methods), '', array_values($pwd_methods)
        ),
        new PasswordAttribute(
          _('Password'), _('Password (Leave empty if you do not wish to change it)'),
          $ldapName.'_password', $required
        ),
        new PasswordAttribute(
          _('Password again'), _('Same password as above, to avoid errors'),
          $ldapName.'_password2', $required
        ),
        new HiddenAttribute(
          $ldapName.'_hash'
        )
      ),
      '', '', $acl, $label
    );
  }

  public function setParent(&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $hash = $this->plugin->config->get_cfg_value('passwordDefaultHash', 'crypt/md5');
      $this->attributes[0]->setDefaultValue($hash);
      if ($this->plugin->config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE') {
        $this->attributes[0]->setValue($hash);
        $this->attributes[0]->setDisabled(TRUE);
      }
    }
  }

  /*! \brief Loads this attribute value from the attrs array
   */
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
    } else {
      $this->setRequired(TRUE);
      $this->attributes[0]->resetToDefault();
      $this->attributes[1]->setRequired(TRUE);
      $this->attributes[2]->setRequired(TRUE);
    }
  }

  function readValues($value)
  {
    $pw_storage = $this->plugin->config->get_cfg_value('passwordDefaultHash', 'crypt/md5');
    if (preg_match ('/^{[^}]+}/', $value)) {
      $tmp = passwordMethod::get_method($value);
      if (is_object($tmp)) {
        $pw_storage = $tmp->get_hash();
      }
    } else {
      if ($value != '') {
        $pw_storage = 'clear';
      }
    }
    return array($pw_storage, '', '', $value);
  }

  function writeValues($values)
  {
    if ($values[1] == '') {
      return $values[3];
    }
    $temp = passwordMethod::get_available_methods();
    $test = new $temp[$values[0]]($this->plugin->config, $this->plugin->dn);
    $test->set_hash($values[0]);
    return $test->generate_hash($values[1]);
  }

  function check()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if ($this->attributes[1]->getValue() != $this->attributes[2]->getValue()) {
      return _('Passwords does not match');
    }
  }
}

class simpleSecurityObject extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('organizationalRole', 'top', 'simpleSecurityObject');

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('Simple security object'),
      'plDescription' => _('Simple security object'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('dsa'),
      'plObjectType'  => array('simpleSecurityObject' => array('name'  => _('Simple security object'),
                                                              'filter'  => 'objectClass=simpleSecurityObject')),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('Simple security object'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('dsaRDN')),
          new HostNameAttribute (
            _('Entry name'), _('Account name'),
            'cn', TRUE
          ),
        )
      ),
      'password' => array(
        'name'  => _('Change password'),
        'attrs' => array(
          new UserPasswordAttribute (
            _('Password'), _('Password'),
            'userPassword', FALSE
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'cn='.$this->cn.','.get_ou('dsaRDN').$this->base;
  }
}

?>
