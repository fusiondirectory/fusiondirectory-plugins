<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2013 FusionDirectory
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

class sogoResource extends simplePlugin
{
  var $mainTab = TRUE;

  var $objectclasses = array('inetOrgPerson','top','CalendarResource');

  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return array(
      'plShortName'   => _('SOGo resource'),
      'plDescription' => _('SOGo resource'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('sogoResource' => array(
        'name'        => _('SOGo resource'),
        'filter'      => 'objectClass=CalendarResource',
        'aclCategory' => 'sogo',
        'ou'          => get_ou('sogoRDN'),
        'icon'        => 'plugins/sogo/images/iconMini.png'
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'main' => array(
        'name'  => _('SOGo resource'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('sogoRDN')),
          new HostNameAttribute (
            _('Entry name'), _('Account name'),
            'uid', TRUE
          ),
          new MailAttribute (
            _('Mail'), _('Mail'),
            'mail', TRUE
          ),
          new HiddenAttribute ('sn'),
          new HiddenAttribute ('cn'),
          new TextAreaAttribute (
            _('description'), _('description'),
            'description', FALSE
          ),
          new IntAttribute (
            _('Multiplebookings'), _('Multiplebookings'),
            'Multiplebookings', FALSE,
            0, FALSE
          ),
          new SelectAttribute (
            _('Kind'), _('Kind'),
            'Kind', FALSE,
            array('location', 'thing', 'group')
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['uid']->setUnique(TRUE);

    $this->attributesAccess['sn']->setValue($this->attributesAccess['uid']->getValue());
    $this->attributesAccess['cn']->setValue($this->attributesAccess['uid']->getValue());
  }

  /* Returns the dn of the entry */
  function compute_dn()
  {
    return 'uid='.$this->attributesAccess['uid']->getValue().','.get_ou('sogoRDN').$this->base;
  }

  function save_object()
  {
    parent::save_object();
    $this->attributesAccess['sn']->setValue($this->attributesAccess['uid']->getValue());
    $this->attributesAccess['cn']->setValue($this->attributesAccess['uid']->getValue());
  }
}

?>
