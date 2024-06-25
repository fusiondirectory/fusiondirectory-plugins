<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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

class sambaSystemTab extends simplePlugin
{
  protected $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Samba'),
      'plDescription' => _('Windows workstation information'),
      'plSmallIcon'   => 'geticon.php?context=applications&icon=samba&size=16',
      'plObjectClass' => ['posixAccount','sambaSamAccount'],
      'plObjectType'  => ['workstation'],
      'plPriority'    => 6,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo ($word = NULL, $rdn = NULL): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new HiddenAttribute('uid'),
          new HiddenAttribute('sambaSID'),
          new HiddenAttribute('homeDirectory', TRUE, '/dev/null'),
          new HiddenAttribute('uidNumber', TRUE, 1000),
          new HiddenAttribute('gidNumber', TRUE, 515),
          new HiddenAttribute('sambaAcctFlags', TRUE, '[W          ]'),
          new SelectAttribute(
            _('Domain'), _('Samba domain name'),
            'sambaDomainName', TRUE
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['uid']->setUnique('whole');
    $domains = sambaDomainManagement::getSambaDomains();
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($domains));
    $this->attributesAccess['sambaDomainName']->setInLdap(FALSE);

    /* Get samba domain */
    if ($this->sambaSID != '') {
      $SID  = preg_replace('/-[^-]+$/', '', $this->sambaSID);
      foreach ($domains as $name => $domain) {
        if ($domain['SID'] == $SID) {
          $this->sambaDomainName = $name;
          break;
        }
      }
    }
  }

  protected function update_uid ()
  {
    $this->attributesAccess['uid']->setValue($this->parent->getBaseObject()->cn.'$');
  }

  function update (): bool
  {
    parent::update();
    $this->update_uid();
    return TRUE;
  }

  function prepare_save (): array
  {
    global $config;
    $this->update_uid();
    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    // Check if the sambaSID exist and that it is correct
    // If not get it from config and generate a new sid

    if (($this->sambaSID == '') || (substr_count($this->sambaSID, '-') < 7)) {
      $domains  = sambaDomainManagement::getSambaDomains();
      $SIDBase  = $domains[$this->sambaDomainName]['SID'];
      $ridBase  = $domains[$this->sambaDomainName]['RIDBASE'];

      // create sambaSID for workstation
      $uidNumber  = $this->uidNumber;
      $ldap       = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      do {
        $sambaSID = $SIDBase.'-'.($uidNumber * 2 + $ridBase);
        $ldap->search('(sambaSID='.$sambaSID.')', ['sambaSID']);
        $uidNumber++;
      } while ($ldap->count() != 0);
      $uidNumber--;
      $this->attrs['sambaSID']  = $sambaSID;
      $this->attrs['uidNumber'] = $uidNumber + $ridBase;
    }

    return $errors;
  }
}
