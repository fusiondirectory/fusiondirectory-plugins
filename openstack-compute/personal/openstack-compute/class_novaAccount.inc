<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011  Yuriy Taraday
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

class novaAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('novaUser');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Nova'),
      'plDescription'   => _('Nova user account settings'),
      'plIcon'          => 'plugins/openstack-compute/images/icon.png',
      'plSmallIcon'     => 'plugins/openstack-compute/images/iconMini.png',
      'plSelfModify'    => TRUE,
      'plPriority'      => 12,
      'plObjectType'    => array('user'),
      'plRequirements'  => array('ldapSchema' => array('novaUser' => ''), 'onFailureDisablePlugin' => array(get_class())),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }


  static function getRandStr($len)
  {
    $chars  = "abcbefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^[]{}&/-=+_";
    $res    = "";
    for ($i = 0; $i < $len; $i++) {
      $res .= $chars[rand(0, strlen($chars) - 1)];
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
    $this->attributesAccess['accessKey']->setDisabled(TRUE);
    $this->attributesAccess['secretKey']->setDisabled(TRUE);
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array (
        'name'  => _('Nova Account'),
        'attrs' => array (
          new BooleanAttribute (
            _('Set as Nova Admin'), _('Set this user as Nova Admin'),
            'isNovaAdmin', FALSE,
            FALSE
          ),
          new StringAttribute (
            _('Access Key'), _('Access Key'),
            'accessKey', TRUE,
            self::getRandStr(36)
          ),
          new StringAttribute (
            _('Secret Key'), _('Secret Key'),
            'secretKey', TRUE,
            self::getRandStr(36)
          )
        )
      )
    );
  }
}
?>
