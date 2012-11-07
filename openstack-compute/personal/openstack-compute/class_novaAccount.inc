<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011  Yuriy Taraday
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

class novaAccount extends simplePlugin
{

  var $displayHeader = TRUE;
  var $plHeadline= "Nova";
  var $plDescription= "Edit users Nova settings";
  var $objectclasses= array("novaUser");

  static function plInfo()
  {
    return (array(
          "plShortName"     => _("Nova"),
          "plDescription"   => _("Edit users Nova settings"),
          "plSelfModify"    => TRUE,
          "plDepends"       => array("user"),
          "plPriority"      => 3,                                 // Position in tabs
          "plSection"     => array("personal" => _("My account")),
          "plCategory"    => array("users"),
          "plRequirements"=> array('ldapSchema' => array('novaUser' => ''), 'onFailureDisablePlugin' => array(get_class())),
      "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
      ));
  }


  function getRandStr($len) {
    $chars = "abcbefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^[]{}&/-=+_";
    $res = "";
    for ($i=0; $i<$len; $i++) {
      $res .= $chars[rand(0,strlen($chars)-1)];
    }
    return $res;
  }

  /*!
   * \brief Constructor of the freeradiusAccount
   *        It will build the select groups choices
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=novaUser)",array('cn'));
    $this->attributesInfo['section1']['attrs']['accessKey']->setDefaultValue(novaAccount::getRandStr(36));
    $this->attributesInfo['section1']['attrs']['accessKey']->resetToDefault();
    $this->attributesInfo['section1']['attrs']['accessKey']->setDisabled(TRUE);
    $this->attributesInfo['section1']['attrs']['secretKey']->setDefaultValue(novaAccount::getRandStr(36));
    $this->attributesInfo['section1']['attrs']['secretKey']->resetToDefault();
    $this->attributesInfo['section1']['attrs']['secretKey']->setDisabled(TRUE);
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
    // Attributes are grouped by section
    'section1' => array (
        'name'  => _('Nova Account'),
        'attrs' => array (
          new BooleanAttribute (
            _('Set as Nova Admin'),              // Label of the attribute
            _('Set as Nova Admin'),              // Description
            'isNovaAdmin',                 // LDAP name
            FALSE,                            // Mandatory
            FALSE                            // default value
          ),
          new StringAttribute (
            _('Access Key'),                     // Label
            _('Access Key'),      // Description
            'accessKey',                 // LDAP name
            TRUE,                           // Mandatory
            ''             // Default value
          ),
          new StringAttribute (
            _('Secret Key'),                     // Label
            _('Secret Key'),      // Description
            'secretKey',                 // LDAP name
            TRUE,                           // Mandatory
            ''             // Default value
          )
        )
      )
    );
  }
}
?>
