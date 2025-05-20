<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannRessource extends simplePlugin
{
  // Plugin info
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Ressource'),
      'plDescription' => _('SupAnn Ressource'),
      'plObjectClass' => ['fdSupannRessource'],
      'plFilter'      => '(objectClass=fdSupannRessource)',
      'plObjectType'  => [
        'supannRessource' => [
          'name'        => _('SupAnn Ressource'),
          'description' => _('SupAnn Ressource'),
          'ou'          => get_ou('SupannRessourceRDN'),
          'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'    => 'fdSupannRessourceName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannRessourceName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  // Define attributes with choices set in constructor
  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Ressource'),
        'attrs' => [
          new StringAttribute(
            _('Ressource Name'), _('The name of the Supann Ressource'),
            'fdSupannRessourceName', TRUE
          ),
          new StringAttribute(
            _('Ressource Label'), _('The label of the Supann Ressource'),
            'fdSupannRessourceLabel', TRUE
          ),
          new StringAttribute(
            _('Ressource Description'), _('The description explaining this Supann Ressource'),
            'description', TRUE
          ),
        ],
      ],
      'states' => [
        'name' => _('Available States'),
        'attrs' => [
          new OrderedArrayAttribute(
            new SelectAttribute(
              _('State'),
              _('Select a state for this resource'),
              'fdSupannAvailableState',
              TRUE,
              [],   // Choices set in constructor
              '',   // No default
              []    // Labels set in constructor
            ),
            FALSE,
            [],
            TRUE   // Multi-valued
          ),
        ],
      ],
    ];
  }

  // Constructor: update the choices using the default OU from config
  public function __construct($dn = null, $object = null, $parent = null, $mainTab = false)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    // If the fdSupannAvailableState attribute exists, populate its choices
    if (
      isset($this->attributesAccess['fdSupannAvailableState']) &&
      ($this->attributesAccess['fdSupannAvailableState']->attribute instanceof SelectAttribute)
    ) {
      $states = self::getAvailableStates();
      $this->attributesAccess['fdSupannAvailableState']->attribute->setChoices(
        array_keys($states),
        array_values($states)
      );
    }
  }

  // Loads states from LDAP only, no defaults. Always uses the configured OU.
  public static function getAvailableStates(): array
  {
    global $config;

    // Build the base DN from our configured OU
    $ou = get_ou('SupannStateRDN') . $config->current['BASE'];

    // Perform the LDAP search
    $ldap = $config->get_ldap_link();
    $ldap->cd($ou);
    $ldap->search('(objectClass=fdSupannRessourceState)', ['fdSupannStateName', 'fdSupannRessourceLabel']);

    $states = [];
    while ($attrs = $ldap->fetch()) {
      if (!empty($attrs['fdSupannStateName'][0])) {
        $key   = $attrs['fdSupannStateName'][0];
        // Use label if present, else fallback to key
        $label = !empty($attrs['fdSupannRessourceLabel'][0]) ? $attrs['fdSupannRessourceLabel'][0] : $key;
        $states[$key] = $label;
      }
    }

    return $states;
  }
}
