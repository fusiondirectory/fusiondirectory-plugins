<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2020  FusionDirectory

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

class opsiClient extends simplePlugin
{
  protected $displayHeader  = TRUE;

  var $inheritance = ['gosaGroupOfNames' => 'member'];

  var $initialMembers;
  var $products;

  protected $initialOpsiId = NULL;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('OPSI client'),
      'plDescription' => _('Edit OPSI client settings'),
      'plSmallIcon'   => 'geticon.php?context=applications&icon=opsi&size=16',
      'plObjectClass' => ['opsiClient'],
      'plObjectType'  => ['workstation', 'ogroup-dynamic'],
      'plDepends'     => ['sambaSystemTab'],
      'plConflicts'   => ['debconfStartup','faiStartup'],
      'plPriority'    => 8,
      'plForeignKeys'   => [
        'fdOpsiServerDn'  => 'serverGeneric',
        'fdOpsiProfileDn' => 'opsiProfile',
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('OPSI Client'),
        'attrs' => [
          new SubmittingOPSIServerAttribute(
            _('OPSI Server'), _('The OPSI Server this client is connected to'),
            'fdOpsiServerDn', TRUE
          ),
          new SelectAttribute(
            _('OPSI Profile'), _('The OPSI Profile to apply to this client'),
            'fdOpsiProfileDn', FALSE
          ),
        ]
      ],
      'products' => [
        'name'  => _('Softwares'),
        'attrs' => [
          new OrderedArrayAttribute(
            new CharSeparatedCompositeAttribute(
              _('The localboot products to setup on this host'),
              'fdOpsiLocalbootProduct',
              [
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_product', TRUE
                ),
                new SelectAttribute(
                  '', '',
                  'fdOpsiLocalbootProduct_action', TRUE,
                  ['setup', 'always', 'once', 'custom', 'userlogin', 'update']
                )
              ],
              '|', '', _('Localboot products')
            ),
            FALSE, // non-ordered
            [],
            TRUE // edition
          ),
          new BooleanAttribute(
            _('Inherit group localboots'), _('If this is checked localboot configured in the group OPSI tab will be installed along with the localboots configured here'),
            'fdOpsiLocalbootInherit', FALSE
          ),
        ]
      ],
      'infos' => [
        'name'  => _('Information'),
        'attrs' => [
          new DisplayAttribute(
            _('Last seen'), _('Last time this OPSI client was seen by the OPSI server'),
            'lastSeen', FALSE
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    if ($object instanceof ogroup) {
      $this->initialMembers = [];
      if (isset($object->saved_attributes['member'])) {
        $this->initialMembers = $object->saved_attributes['member'];
        if (!is_array($this->initialMembers)) {
          $this->initialMembers = [$this->initialMembers];
        }
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setSubmitForm('product_changed');
    $this->attributesAccess['lastSeen']->setAllowHTML(TRUE);
    if ($this->initialOpsiId === NULL) {
      // If we have no OPSI id avoid storing one from a future server change
      $this->initialOpsiId = FALSE;
    }
    if (!$this->member_of_group) {
      $this->attributesAccess['fdOpsiLocalbootInherit']->setVisible(FALSE);
    } else {
      $this->attributesAccess['fdOpsiLocalbootInherit']->setSubmitForm(TRUE);
    }
  }

  public function update (): bool
  {
    parent::update();
    if ($this->member_of_group) {
      if ($this->fdOpsiLocalbootInherit) {
        $this->fdOpsiServerDn = $this->group_attrs['fdOpsiServerDn'][0];
        $this->attributesAccess['fdOpsiServerDn']->setDisabled(TRUE);
      } else {
        $this->attributesAccess['fdOpsiServerDn']->setDisabled(FALSE);
      }
    }
    return TRUE;
  }

  function prepare_save (): array
  {
    if ($this->member_of_group && (($this->fdOpsiProfileDn == 'inherited') || $this->fdOpsiLocalbootInherit)) {
      $this->fdOpsiServerDn = 'inherited';
    }
    return parent::prepare_save();
  }

  function save (): array
  {
    global $config;
    if ($this->initially_was_account && $this->editing_group() && !$this->is_template) {
      $oldMembers = array_diff($this->initialMembers, $this->parent->getBaseObject()->member);
      $macAddresses = [];
      $ldap = $config->get_ldap_link();
      foreach ($oldMembers as $member) {
        $ldap->cat($member, ['macAddress'], '(&(!(objectClass=opsiClient))(macAddress=*))');
        if ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        }
      }
      if (!empty($macAddresses)) {
        $ldap->cat($this->fdOpsiServerDn, ['macAddress']);
        if (($attrs = $ldap->fetch()) && isset($attrs['macAddress'][0])) {
          $s_daemon   = new supportDaemon();
          if ($s_daemon->is_available()) {
            $s_daemon->append_call('OPSI.delete', [$attrs['macAddress'][0]], ['args' => [$macAddresses]]);
            if ($s_daemon->is_error()) {
              $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
              $error->display();
            }
          }
        } else {
          $error = new FusionDirectoryError(htmlescape(_('Could not get macAddress for OPSI server!')));
          $error->display();
        }
      }
    }

    $errors = parent::save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($this->is_account && !$this->is_template) {
      $this->update_or_insert();
    }

    return $errors;
  }

  function getMacAddresses ()
  {
    global $config;
    $macAddresses = [];
    if ($this->editing_group()) {
      $ldap = $config->get_ldap_link();
      foreach ($this->parent->getBaseObject()->member as $member) {
        $ldap->cat($member, ['macAddress'], '(macAddress=*)');
        if ($attrs = $ldap->fetch()) {
          $macAddresses[] = $attrs['macAddress'][0];
        } else {
          if ($ldap->success()) {
            $errorText = _('This member was not found or has no macAddress');
          } else {
            $errorText = $ldap->get_error();
          }
          $error = new FusionDirectoryError(
            nl2br(sprintf(
              htmlescape(_("Could not get macAddress for \"%s\":\n%s")),
              htmlescape($member),
              htmlescape($errorText)
            ))
          );
          $error->display();
        }
      }
    } else {
      $macAddresses = $this->parent->getBaseObject()->macAddress;
      if (!is_array($macAddresses)) {
        $macAddresses = [$macAddresses];
      } elseif (count($macAddresses) > 1) {
        $macAddresses = [reset($macAddresses)];
      }
    }
    return $macAddresses;
  }

  function update_or_insert ()
  {
    global $config;
    $s_daemon     = new supportDaemon();
    $macAddresses = $this->getMacAddresses();
    if (!empty($macAddresses) && $s_daemon->is_available()) {
      if (!$this->editing_group() && !empty($this->initialOpsiId)) {
        $s_daemon->append_call('OPSI.update_or_insert', reset($macAddresses), ['args' => [$this->initialOpsiId]]);
      } else {
        $s_daemon->append_call('OPSI.update_or_insert', $macAddresses, []);
      }
      if ($s_daemon->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
      }
    }
  }

  function remove (bool $fulldelete = FALSE): array
  {
    if ($this->member_of_group && !$this->is_template) {
      // If the winstation is member of an ogroup which has OPSI activated, we must update_or_insert information
      // We have to call this in here because save won't be called as is_account is FALSE
      $this->update_or_insert();
    }

    return parent::remove($fulldelete);
  }

  protected function ldap_remove (): array
  {
    global $config;

    if (!$this->is_template) {
      $macAddresses = [];
      if ($this->editing_group()) {
        $ldap = $config->get_ldap_link();
        foreach ($this->initialMembers as $member) {
          $ldap->cat($member, ['macAddress'], '(&(!(objectClass=opsiClient))(macAddress=*))');
          if ($attrs = $ldap->fetch()) {
            $macAddresses[] = $attrs['macAddress'][0];
          }
        }
      } elseif (!$this->member_of_group) {
        $macAddresses = $this->parent->getBaseObject()->macAddress;
        if (!is_array($macAddresses)) {
          $macAddresses = [$macAddresses];
        } elseif (count($macAddresses) > 1) {
          $macAddresses = [$macAddresses[0]];
        }
      }

      if (!empty($macAddresses)) {
        $s_daemon   = new supportDaemon();
        if ($s_daemon->is_available()) {
          $s_daemon->append_call('OPSI.delete', $macAddresses, []);
          if ($s_daemon->is_error()) {
            $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
            $error->display();
          }
        }
      }
    }

    return parent::ldap_remove();
  }

  public function server_changed ()
  {
    global $config;

    $fdOpsiServerDn = $this->fdOpsiServerDn;
    if ($fdOpsiServerDn == 'inherited') {
      $fdOpsiServerDn = $this->group_attrs['fdOpsiServerDn'][0];
    }

    $ldap = $config->get_ldap_link();
    $ldap->search('(&(objectClass=opsiProfile)(fdOpsiServerDn='.$fdOpsiServerDn.'))', ['cn']);
    $profilesdn    = [];
    $profileslabel = [];
    while ($attrs = $ldap->fetch()) {
      $profilesdn[]    = $attrs['dn'];
      $profileslabel[] = $attrs['cn'][0];
    }
    if ($this->member_of_group && isset($this->group_attrs['fdOpsiProfileDn'][0])) {
      $profilesdn[]     = 'inherited';
      $profileslabel[]  = sprintf(_('Inherited (%s)'), ldap_explode_dn($this->group_attrs['fdOpsiProfileDn'][0], 1)[0]);
      $this->attributesAccess['fdOpsiProfileDn']->setManagedAttributes(
        [
          'disable' => [
            'inherited' => ['fdOpsiServerDn']
          ]
        ]
      );
    }
    $this->attributesAccess['fdOpsiProfileDn']->setChoices($profilesdn, $profileslabel);

    $s_daemon = new supportDaemon();
    if (!$s_daemon->is_available()) {
      $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
      $error->display();
      return;
    }
    try {
      $macAddress = opsiProfile::getOpsiServerMac($fdOpsiServerDn);
    } catch (FusionDirectoryError $error) {
      $error->display();
      return FALSE;
    }
    $macAddresses = $this->getMacAddresses();
    if (!empty($macAddresses)) {
      $infos = $s_daemon->append_call('OPSI.host_getObjects', $macAddress, ['args' => [['lastSeen', 'hardwareAddress'], ['hardwareAddress' => $macAddresses]]]);
      if ($s_daemon->is_error()) {
        $this->lastSeen = sprintf(_('Failed to contact OPSI server: %s'), $s_daemon->get_error());
      } elseif (count($infos) == 0) {
        $this->lastSeen = _('Never');
      } else {
        if (!$this->editing_group() && ($this->initialOpsiId === NULL)) {
          $this->initialOpsiId = reset($infos)['id'];
        }
        $text = '';
        foreach ($infos as $info) {
          $text .= htmlescape(sprintf('%s - %s', $info['id'], $info['lastSeen']))."<br/>\n";
        }
        $this->lastSeen = $text;
      }
    }
    if (!isset($this->products[$this->fdOpsiServerDn])) {
      $opsi_args  = ['id','name','setupScript','alwaysScript','onceScript','customScript','userLoginScript','updateScript','productVersion','packageVersion'];
      $localboots = $s_daemon->append_call('OPSI.get_localboots', $macAddress, ['args' => [$opsi_args]]);
      if ($s_daemon->is_error()) {
        $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
        $error->display();
        return;
      }
      $this->products[$this->fdOpsiServerDn]['localboots'] = [];
      foreach ($localboots as $localboot) {
        $this->products[$this->fdOpsiServerDn]['localboots'][$localboot['id']] = $localboot;
      }
    }
    if ($s_daemon->is_error()) {
      $error = new FusionDirectoryError(msgPool::siError($s_daemon->get_error()));
      $error->display();
      return;
    }
    $choices = [];
    foreach ($this->products[$this->fdOpsiServerDn]['localboots'] as $id => $infos) {
      $choices[$id] = sprintf(_('%s (%s-%s)'), $infos['id'], $infos['productVersion'], $infos['packageVersion']);
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->setChoices(
      array_keys($choices),
      array_values($choices)
    );
  }

  public function product_changed ()
  {
    $localboot = $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[0]->getValue();

    $actions = [];
    foreach (['setup', 'always', 'once', 'custom', 'userLogin', 'update'] as $action) {
      if (!empty($this->products[$this->fdOpsiServerDn]['localboots'][$localboot][$action.'Script'])) {
        $actions[] = strtolower($action);
      }
    }
    $this->attributesAccess['fdOpsiLocalbootProduct']->attribute->attributes[1]->setChoices($actions);
  }
}
