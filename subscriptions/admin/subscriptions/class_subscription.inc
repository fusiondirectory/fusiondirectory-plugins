<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2020-2021 FusionDirectory

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

class subscription extends simplePlugin
{
  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Subscription'),
      'plDescription' => _('Subscription'),
      'plObjectClass' => ['fdSubscription'],
      'plObjectType'  => ['subscription' => [
        'name'        => _('Subscription'),
        'icon'        => 'geticon.php?context=types&icon=subscription&size=16',
        'ou'          => get_ou('subscriptionRDN'),
      ]],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  public static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Information'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('subscriptionRDN')),
          new HostNameAttribute(_('Name'), _('Name to identify this subscription'), 'cn', TRUE),
          new TextAreaAttribute(
            _('Description'), _('Description of this subscription'),
            'description', FALSE
          ),
          new SelectAttribute(
            _('Type'), _('Subscription type'),
            'fdSubscriptionType', FALSE
          ),
          new StringAttribute(
            _('Contract'), _('Contract identifier from Dolibarr'),
            'fdSubscriptionContractId', FALSE
          ),
        ]
      ],
      'login' => [
        'name'  => _('Login'),
        'attrs' => [
          new UidAttribute(
            _('Login'), _('Login for this subscription'),
            'uid', TRUE
          ),
          new PasswordAttribute(
            _('Password'), _('Password'),
            'userPassword', FALSE
          ),
        ]
      ],
      'status' => [
        'name'  => _('Status'),
        'attrs' => [
          new BooleanAttribute(
            _('Active'), _('Whether this subscription is active'),
            'fdSubscriptionActive', FALSE
          ),
          new GeneralizedTimeDateAttribute(
            _('Start date'), _('Start date of this subscription'),
            'fdSubscriptionStartDate', FALSE
          ),
          new GeneralizedTimeDateAttribute(
            _('End date'), _('End date of this subscription'),
            'fdSubscriptionEndDate', FALSE
          ),
        ]
      ],
      'export' => [
        'name'  => _('Export'),
        'attrs' => [
          new ButtonAttribute(
            _('Subscription file'), _('Generate the subscription file to send to the client'),
            'generateSubscriptionFile', _('Generate')
          ),
        ]
      ],
    ];
  }

  public function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['generateSubscriptionFile']->setInLdap(FALSE);
    foreach ($config->get_cfg_value('SubscriptionTypes', []) as $subscriptionType) {
      list($type, $label) = explode(';', $subscriptionType);
      $types[]      = $type;
      $typeLabels[] = $label;
    }
    $this->attributesAccess['fdSubscriptionType']->setChoices($types, $typeLabels);
  }

  public function handle_generateSubscriptionFile ()
  {
    $fileName     = 'fusiondirectory-subscription-'.strtolower($this->cn).'.ini';
    $fileContent  = "[FusionDirectory Subscription]\n";
    foreach (static::$subscriptionAttributes as $attr => $iniVar) {
      if ($attr === 'fdSubscriptionName') {
        $fileContent .= $iniVar.'='.$this->cn."\n";
      } else {
        $fileContent .= $iniVar.'='.$this->$attr."\n";
      }
    }

    session::set('binary', $fileContent);
    session::set('binarytype', 'octet-stream');
    session::set('binaryfile', $fileName);
    header('location: getbin.php');
    exit();
  }
}
