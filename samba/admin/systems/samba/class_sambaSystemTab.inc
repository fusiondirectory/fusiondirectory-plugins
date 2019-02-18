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
  var $displayHeader = TRUE;

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Samba'),
      'plDescription' => _('Windows workstation information'),
      'plObjectClass' => array('posixAccount','sambaSamAccount'),
      'plObjectType'  => array('workstation'),
      'plPriority'    => 6,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ($word = NULL, $rdn = NULL): array
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
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
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['uid']->setUnique('whole');
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($config->data['SERVERS']['SAMBA']));
    $this->attributesAccess['sambaDomainName']->setInLdap(FALSE);

    // Get samba domain and its sid/rid base
    if ($this->sambaSID != '') {
      $this->SID = preg_replace ('/-[^-]+$/', '', $this->sambaSID);
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->search('(&(objectClass=sambaDomain)(sambaSID='.$this->SID.'))', array('sambaAlgorithmicRidBase','sambaDomainName'));
      if ($ldap->count() != 0) {
        $attrs = $ldap->fetch();
        if (isset($attrs['sambaAlgorithmicRidBase'])) {
          $this->ridBase = $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase = $config->get_cfg_value('sambaRidBase');
        }
        if ($this->sambaDomainName == '') {
          $this->sambaDomainName = $attrs['sambaDomainName'][0];
        }
      } else {
        // Fall back to a 'DEFAULT' domain, if none was found in LDAP.
        if ($this->sambaDomainName == '') {
          $this->sambaDomainName = 'DEFAULT';
        }

        // Nothing in ldap, use configured sid and rid values.
        $this->ridBase  = $config->get_cfg_value('sambaRidBase');
        $this->SID      = $config->get_cfg_value('sambaSid');
      }
    }
  }

  protected function update_uid()
  {
    $this->attributesAccess['uid']->setValue($this->parent->getBaseObject()->cn.'$');
  }

  function save_object ()
  {
    parent::save_object();
    $this->update_uid();
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
      $this->SID      = $config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      $this->ridBase  = $config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];

      // create sambaSID for workstation
      $uidNumber = $this->uidNumber;
      do {
        $sid = $this->SID.'-'.($uidNumber * 2 + $this->ridBase);
        $ldap = $config->get_ldap_link();
        $ldap->cd($config->current['BASE']);
        $ldap->search('(sambaSID='.$sid.')', array('sambaSID'));
        $uidNumber++;
      } while ($ldap->count() != 0);
      $uidNumber--;
      $this->attrs['sambaSID']  = $sid;
      $this->attrs['uidNumber'] = $uidNumber + $this->ridBase;
    }

    return $errors;
  }
}
?>
