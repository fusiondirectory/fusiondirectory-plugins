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
      'plSearchAttrs' => ['cn','description','fdSubscriptionContractId'],

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
            'fdSubscriptionType', TRUE
          ),
          new StringAttribute(
            _('Contract'), _('Contract identifier from Dolibarr'),
            'fdSubscriptionContractId', TRUE
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
            'userPassword', TRUE
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
            'fdSubscriptionStartDate', TRUE
          ),
          new GeneralizedTimeDateAttribute(
            _('End date'), _('End date of this subscription'),
            'fdSubscriptionEndDate', TRUE
          ),
        ]
      ],
      'repositories' => [
        'name'  => _('Repositories'),
        'attrs' => [
          new SelectAttribute(
            _('Apt repository'), _('Apt repository'),
            'fdSubscriptionAptRepository', TRUE
          ),
          new SelectAttribute(
            _('RPM distribution'), _('RPM distribution'),
            'fdSubscriptionRpmRepository', TRUE
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
          new ButtonAttribute(
            _('APT auth.conf'), _('Generate the auth.conf file to send to the client if he uses Debian or a derivate'),
            'generateAptAuthFile', _('Generate')
          ),
          new ButtonAttribute(
            _('APT'), _('Generate the fusiondirectory-subscription.list file to send to the client if he uses an Apt based distribution'),
            'generateAptFile', _('Generate')
          ),
          new ButtonAttribute(
            _('RPM'), _('Generate the fusiondirectory-subscription.repo file to send to the client if he uses an RPM based distribution'),
            'generateRpmFile', _('Generate')
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
    $this->attributesAccess['generateAptFile']->setInLdap(FALSE);
    $this->attributesAccess['generateRpmFile']->setInLdap(FALSE);
    $types      = [];
    $typeLabels = [];
    foreach ($config->get_cfg_value('SubscriptionTypes', []) as $subscriptionType) {
      list($type, $label) = explode(';', $subscriptionType, 2);
      $types[]      = $type;
      $typeLabels[] = $label;
    }
    $this->attributesAccess['fdSubscriptionType']->setChoices($types, $typeLabels);
    $aptRepos       = [];
    $aptRepoLabels  = [];
    foreach ($config->get_cfg_value('SubscriptionAptRepositories', []) as $aptRepository) {
      list($repo, $label) = explode(';', $aptRepository, 2);
      $aptRepos[]       = $repo;
      $aptRepoLabels[]  = $label;
    }
    $this->attributesAccess['fdSubscriptionAptRepository']->setChoices($aptRepos, $aptRepoLabels);
    $rpmRepos       = [];
    $rpmRepoLabels  = [];
    foreach ($config->get_cfg_value('SubscriptionRpmRepositories', []) as $rpmRepository) {
      list($repo, $label) = explode(';', $rpmRepository, 2);
      $rpmRepos[]       = $repo;
      $rpmRepoLabels[]  = $label;
    }
    $this->attributesAccess['fdSubscriptionRpmRepository']->setChoices($rpmRepos, $rpmRepoLabels);
  }

  public function handle_generateSubscriptionFile ()
  {
    $fileName     = 'fusiondirectory-subscription-'.strtolower($this->cn).'.ini';
    $fileContent  = "[FusionDirectory Subscription]\n";
    foreach (subscriptionInfo::$subscriptionAttributes as $attr => $iniVar) {
      if ($attr === 'fdSubscriptionName') {
        $fileContent .= $iniVar.'='.$this->cn."\n";
      } else {
        $fileContent .= $iniVar.'='.$this->$attr."\n";
      }
    }
    send_binary_content($fileContent, $fileName);
  }

  public function handle_generateAptAuthFile ()
  {
    global $config;

    $fileName     = strtolower($this->cn).'-auth.conf';
    $fileContent  = 'machine '.$config->get_cfg_value('SubscriptionHost')."\n";
    $fileContent  .= ' login '.$this->uid."\n";
    $fileContent  .= ' password '.$this->userPassword."\n";
    send_binary_content($fileContent, $fileName);
  }

  public function handle_generateAptFile ()
  {
    global $config;

    $label  = $this->attributesAccess['fdSubscriptionAptRepository']->displayValue($this->fdSubscriptionAptRepository);
    $name   = strtolower(preg_replace(['/\s/','/[^[:alnum:]]/'], ['-',''], $label));

    $fileName     = strtolower($this->cn).'-fusiondirectory-subscription-'.$name.'.list';
    $fileContent  = 'deb https://'.$config->get_cfg_value('SubscriptionHost').'/'.$this->fdSubscriptionAptRepository.' main'."\n";

    send_binary_content($fileContent, $fileName);
  }

  public function handle_generateRpmFile ()
  {
    global $config;

    $label  = $this->attributesAccess['fdSubscriptionRpmRepository']->displayValue($this->fdSubscriptionRpmRepository);
    $name   = strtolower(preg_replace(['/\s/','/[^[:alnum:]]/'], ['-',''], $label));

    $fileName     = strtolower($this->cn).'-fusiondirectory-subscription-'.$name.'.repo';
    $fileContent  = '[fusiondirectory-subscription-'.$name.']'."\n";
    $fileContent  .= 'name=Fusiondirectory Packages for '.$label."\n";
    $fileContent  .= 'baseurl=https://'.$this->uid.':'.$this->userPassword.'@'.$config->get_cfg_value('SubscriptionHost').'/'.$this->fdSubscriptionRpmRepository."\n";
    $fileContent  .= 'enabled=1'."\n";
    $fileContent  .= 'gpgcheck=1'."\n";
    send_binary_content($fileContent, $fileName);
  }
}
