<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

class dhcpHost extends dhcpPlugin
{
  private $parentPlugin;

  public static function plInfo (): array
  {
    return [
      'plShortName'     => _('DHCP host'),
      'plDescription'   => _('DHCP host'),
      'plCategory'      => ['dhcpConfiguration'],
      'plObjectClass'   => ['dhcpHost'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    $attributesInfo = parent::getAttributesInfo();
    $attributesInfo['host'] = [
      'name'  => _('Host'),
      'attrs' => [
        new CompositeAttribute(
          _('The client hardware address'), 'dhcpHWAddress',
          [
            new SelectAttribute(
              '', _('The hardware address type'),
              'dhcpHWAddress_type', TRUE,
              [  'ethernet',   'fddi',   'token-ring'], 'ethernet',
              [_('Ethernet'),_('FDDI'),_('Token Ring')]
            ),
            new StringAttribute(
              '', _('The client hardware address'),
              'dhcpHWAddress_address', TRUE
            ),
          ],
          '/^([^ ]+) ([^ ]+)$/',
          '%s %s',
          '',
          _('Hardware address')
        )
      ]
    ];
    $attributesInfo['host']['attrs'][0]->setRequired(TRUE);
    $attributesInfo['host']['attrs'][0]->setLinearRendering(TRUE);
    $attributesInfo['main']['attrs'][2]->setDefaultValue(
      [
        'fixed-address ip',
      ]
    );
    return $attributesInfo;
  }

  function __construct ($plugin, $dn, $attrs = NULL)
  {
    $this->parentPlugin = $plugin;
    parent::__construct($plugin, $dn, $attrs);
  }

  function check (): array
  {
    $errors = parent::check();
    $dn = $this->dn;
    if (($dn == '') || ($dn == 'new')) {
      $dn = $this->compute_dn();
    }

    foreach ($this->dhcpStatements as $statement) {
      if (preg_match('/^fixed-address (.+)$/', $statement, $m)) {
        if (!tests::is_ip($m[1])) {
          $errors[] = new SimplePluginCheckError(
            $this,
            htmlescape(sprintf(_('"%s" is not a valid IP address.'), $m[1]))
          );
        } else {
          $error = $this->parentPlugin->attributesAccess['dhcpSections']->isIpInParentSubnet($dn, $m[1]);
          if ($error !== TRUE) {
            $errors[] = new SimplePluginCheckError(
              $this,
              htmlescape($error)
            );
          }
        }
      }
    }

    return $errors;
  }
}
