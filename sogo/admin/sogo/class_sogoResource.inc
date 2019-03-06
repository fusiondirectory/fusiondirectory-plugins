<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2013-2016 FusionDirectory

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

class sogoResource extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('SOGo resource'),
      'plDescription' => _('SOGo resource'),
      'plObjectClass' => ['inetOrgPerson','CalendarResource'],
      'plFilter'      => '(objectClass=CalendarResource)',
      'plObjectType'  => ['sogoResource' => [
        'name'        => _('SOGo resource'),
        'aclCategory' => 'sogo',
        'ou'          => get_ou('sogoRDN'),
        'icon'        => 'geticon.php?context=applications&icon=sogo&size=16',
        'mainAttr'    => 'uid'
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  // The main function : information about attributes
  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'main' => [
        'name'  => _('SOGo resource'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('sogoRDN')),
          new HostNameAttribute(
            _('Entry name'), _('Account name'),
            'uid', TRUE
          ),
          new MailAttribute(
            _('Mail'), _('Mail'),
            'mail', TRUE
          ),
          new HiddenAttribute('sn'),
          new HiddenAttribute('cn'),
          new TextAreaAttribute(
            _('description'), _('description'),
            'description', FALSE
          ),
          new IntAttribute(
            _('Multiplebookings'), _('Multiplebookings'),
            'Multiplebookings', FALSE,
            0, FALSE
          ),
          new SelectAttribute(
            _('Kind'), _('Kind'),
            'Kind', FALSE,
            ['location', 'thing', 'group']
          ),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['uid']->setUnique('one');

    $this->attributesAccess['sn']->setValue($this->attributesAccess['uid']->getValue());
    $this->attributesAccess['cn']->setValue($this->attributesAccess['uid']->getValue());
  }

  function save_object ()
  {
    parent::save_object();
    $this->attributesAccess['sn']->setValue($this->attributesAccess['uid']->getValue());
    $this->attributesAccess['cn']->setValue($this->attributesAccess['uid']->getValue());
  }
}
