<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2013  FusionDirectory

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

class DebconfReleaseAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $choices = array())
  {
    parent::__construct($label, $description, $ldapName, $required, $choices);
    $this->setSubmitForm(TRUE);
  }

  function setValue ($value)
  {
    if (!in_array($value, $this->choices)) {
      return;
    }
    parent::setValue($value);
    if (isset($this->plugin)) {
      $this->plugin->attributesInfo['main']['attrs']['gotoBootKernel']->setRelease($value);
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    //~ $this->setChoices(array_keys($this->plugin->cache['SERVERS']));
    $this->setValue('squeeze');
  }
}

class debconfStartup extends simplePlugin
{
  var $displayHeader = TRUE;

  /* attribute list for save action */
  var $objectclasses        = array("debconfStartup");

  var $inheritance = array("gosaGroupOfNames" => "member");

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Debconf settings'),
        'attrs' => array(
          new SelectAttribute (
            _('Profile'), _('Debconf preseed profile to be used for installation'),
            'debconfProfile', TRUE
          ),
          new DebconfReleaseAttribute (
            _('Release'), _('Debian release to install'),
            'debconfDebianRelease', TRUE,
            array('squeeze', 'unstable', 'wheezy')
          ),
          new BootKernelAttribute (
            _('Boot kernel'),  _('Linux kernel to be installed and booted'),
            'gotoBootKernel', FALSE
          ),
          new StringAttribute (
            _('Boot kernel parameters'), _('Linux kernel parameters to be ran'),
            'gotoKernelParameters', FALSE
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    /* Fill profile select box */
    $ldap = $this->config->get_ldap_link();
    $ldap->ls(
      "objectClass=organizationalUnit",
      get_ou("debconfRDN").$this->config->current['BASE'],
      array("ou")
    );
    $profiles     = array();
    while ($attrs = $ldap->fetch()) {
      $profiles[]     = $attrs['ou'][0];
    }
    $this->attributesInfo['main']['attrs']['debconfProfile']->setChoices($profiles);
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Debconf'),
      'plDescription'   => _('Debconf preseed startup'),
      'plSelfModify'    => FALSE,
      'plPriority'      => 9,
      'plObjectType'    => array('workstation', 'server', 'ogroup'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
