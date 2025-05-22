<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2025  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class supannRessourceState extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('State'),
      'plDescription' => _('SupAnn State'),
      'plObjectClass' => ['fdSupannRessourceState'],
      'plFilter'      => '(objectClass=fdSupannRessourceState)',
      'plObjectType'  => ['supannRessourceState' =>
        [
        'name'        => _('SupAnn State'),
        'description' => _('SupAnn State'),
        'ou'          => get_ou('SupannStateRDN'),
        'icon'        => 'geticon.php?context=applications&icon=supann-entite&size=16',
        'mainAttr'    => 'fdSupannStateName',
        ]
       ],
      'plSearchAttrs' => ['description', 'fdSupannStateName'],

      'plForeignKeys' => [
        // Key is the local attribute in this class that stores the DNs
        'fdSupannSubStateRefs' => [
          // This definition specifies what 'fdSupannSubStateRefs' can point to
          [
            'supannRessourceSubState', // The objectClass of the referenced object
            'dn',                        // The reference is to the DN of the target object
            'fdSupannSubStateRefs=%oldvalue%', // A valid LDAP filter for the referenced objects
            '*'                          // Attributes to display from the referenced object
          ],
        ]
      ],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE)
    ];
  }

  /**
   * Get the attributes information for the plugin
   *
   * @return array
   */
  static function getAttributesInfo (): array
  {
    $substates = self::getAvailableSubStates();
    return [
      'main' => [
        'name'  => _('Supann Ressource State'),
        'attrs' => [
          new StringAttribute(
            _('State Name'), _('The name of the Supann State'),
            'fdSupannStateName', TRUE
          ),
          new StringAttribute(
            _('State Label'), _('The label of the Supann State'),
            'fdSupannRessourceLabel', TRUE
          ),
          new StringAttribute(
            _('State Description'), _('The description explaining this Supann State'),
            'description', TRUE
          ),
        ]
      ],
      'substates' => [
        'name'  => _('Available Sub States'),
        'attrs' => [
          new OrderedArrayAttribute(
            new SelectAttribute(
              _('Available Sub States'),
              _('Select one or more available Sub States for this State'),
              'fdSupannSubStateList',
              FALSE,
              array_keys($substates), // DNs as values
              '',
              array_values($substates) // Labels
            ),
            FALSE,
            [],
            TRUE // Multi-valued
          ),
          // Hidden attribute for references
          new HiddenArrayAttribute(
            'fdSupannSubStateRefs', // LDAP attribute name
            FALSE                   // required
          ),
        ]
      ],
    ];
  }

  /**
   * Get the available sub states from LDAP
   *
   * @return array
   */
  public static function getAvailableSubStates(): array
  {
    global $config;
    $ou = get_ou('SupannSubStateRDN') . $config->current['BASE'];
    $ldap = $config->get_ldap_link();
    $ldap->cd($ou);
    $ldap->search('(objectClass=fdSupannRessourceSubState)', ['dn', 'fdSupannRessourceLabel']);

    $substates = [];
    while ($attrs = $ldap->fetch()) {
      if (!empty($attrs['dn'])) {
        $dn = $attrs['dn'];
        $label = !empty($attrs['fdSupannRessourceLabel'][0]) ? $attrs['fdSupannRessourceLabel'][0] : $dn;
        $substates[$dn] = $label;
      }
    }
    return $substates;
  }

  /**
   * Prepare save operation
   *
   * @return array
   */
  protected function prepare_save (): array
  {
    // Call parent to fill $this->attrs as usual
    $errors = parent::prepare_save();

    // Sync the hidden refs attribute with the selected substates
    if (isset($this->attributesAccess['fdSupannSubStateList'])) {
      $selected = $this->attributesAccess['fdSupannSubStateList']->getValue();
      $this->attrs['fdSupannSubStateRefs'] = is_array($selected) ? $selected : [];
    }

    return $errors;
  }
}
