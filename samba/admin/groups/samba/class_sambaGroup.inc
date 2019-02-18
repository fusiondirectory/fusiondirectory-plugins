<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class sambaGroup extends simplePlugin
{
  var $displayHeader  = TRUE;

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Samba'),
      'plDescription' => _('Samba group settings'),
      'plFilter'      => '(objectClass=sambaGroupMapping)',
      'plIcon'        => 'geticon.php?context=applications&icon=samba&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=samba&size=16',
      'plObjectClass' => array('sambaGroupMapping'),
      'plObjectType'  => array('group'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    return array(
      'main' => array(
        'name'  => _('Domain'),
        'attrs' => array(
          new SelectAttribute(
            _('Domain'), _('Samba domain'),
            'sambaDomainName', TRUE
          ),
          new CompositeAttribute(
            _('Samba information'),
            'sambaSID',
            array(
              new HiddenAttribute(
                'SID'
              ),
              new SelectAttribute(
                _('Group type'), _('Samba group type'),
                'GID', TRUE,
                array(0,                512,                513,                514),
                0,
                array(_("Samba group"), _("Domain admins"), _("Domain users"),  _("Domain guests"))
              )
            ),
            '/^([^ ]+)-([0-9]+)$/',
            '%s-%d'
          ),
          new HiddenAttribute(
            'sambaGroupType', TRUE,
            2
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    // User wants me to fake the idMappings? This is useful for
    //  making winbind resolve the user names in a reasonable amount
    //  of time in combination with larger databases.
    if ($config->get_cfg_value('sambaidmapping') == 'TRUE') {
      $this->objectclasses[] = 'sambaIdmapEntry';
    }

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['sambaDomainName']->setInLdap(FALSE);
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($config->data['SERVERS']['SAMBA']));

    /* Get samba Domain in case of samba 3 */
    if ($this->sambaSID != "") {
      /* Get domain name for SID */
      $this->sambaDomainName = "DEFAULT";

      foreach ($config->data['SERVERS']['SAMBA'] as $key => $val) {
        if ($val['SID'] == $this->getSID()) {
          $this->sambaDomainName = $key;
          break;
        }
      }
    } else {
      if ($config->get_cfg_value("sid") != "") {
        $this->sambaDomainName  = "DEFAULT";
        $this->setSID($config->get_cfg_value("sid"));
      } else {
        msg_dialog::display(_("Configuration error"), _("Cannot find group SID in your configuration!"), ERROR_DIALOG);
      }
    }

    $this->attributesAccess['sambaDomainName']->setInitialValue($this->sambaDomainName);

    if ($this->getGID() < 500 || $this->getGID() > 553) {
      $this->setGID(0);
    }

    /* Don't lose special groups! If not key'ed above, add it to the combo box */
    if (!in_array($this->getGID(), $this->attributesAccess['sambaSID']->attributes[1]->getChoices())) {
      $this->attributesAccess['sambaSID']->attributes[1]->setChoices(
        array(0,                512,                513,                514,                $this->getGID()),
        array(_("Samba group"), _("Domain admins"), _("Domain users"),  _("Domain guests"), sprintf(_("Special group (%d)"), $this->getGID()))
      );
    }
    $this->attributesAccess['sambaSID']->attributes[1]->setInitialValue($this->getGID());
  }

  function prepare_save (): array
  {
    global $config;
    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    // Generate rid / primaryGroupId
    if (!isset($config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'])) {
      msg_dialog::display(_("Warning"), _("Undefined Samba SID detected. Please fix this problem manually!"), WARNING_DIALOG);
    } else {
      $this->setSID($config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID']);
    }

    if ($this->getGID() == 0) {
      if (($this->getSID() == "") ||
          $this->attributesAccess['sambaDomainName']->hasChanged() ||
          $this->attributesAccess['sambaSID']->attributes[1]->hasChanged()) {
        $sid                      = $this->getSambaSID();
        $this->attrs['sambaSID']  = $sid;
        $this->sambaSID           = $sid;
      } else {
        unset($this->attrs['sambaSID']);
      }
    } else {
      $this->attrs['sambaSID'] = $this->sambaSID;
    }

    return $errors;
  }

  /*! \brief Get a new SambaSID for a group */
  function getSambaSID()
  {
    global $config;
    $ldap       = $config->get_ldap_link();
    if (class_available('mixedGroup')) {
      $gidNumber  = $this->parent->by_object['mixedGroup']->gidNumber;
    } else {
      $gidNumber  = $this->parent->by_object['posixGroup']->gidNumber;
    }
    do {
      $sid = $this->getSID()."-".($gidNumber * 2 + $this->getRidBase() + 1);
      $ldap->cd($config->current['BASE']);
      $ldap->search("(sambaSID=$sid)", array("sambaSID"));
      $gidNumber++;
    } while ($ldap->count() != 0);

    return $sid;
  }

  function resetCopyInfos ()
  {
    parent::resetCopyInfos();

    /* Set a new SID */
    $this->setSID('');
    $this->attributesAccess['sambaDomainName']->setInitialValue('');
  }

  function getRidBase()
  {
    global $config;
    return $config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];
  }

  function getSID()
  {
    return $this->attributesAccess['sambaSID']->attributes[0]->getValue();
  }

  function setSID($value)
  {
    $this->attributesAccess['sambaSID']->attributes[0]->setValue($value);
  }

  function getGID()
  {
    return $this->attributesAccess['sambaSID']->attributes[1]->getValue();
  }

  function setGID($value)
  {
    $this->attributesAccess['sambaSID']->attributes[1]->setValue($value);
  }

}
?>
