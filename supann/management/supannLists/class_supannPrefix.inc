<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2026  FusionDirectory

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

class supannPrefix extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('SupAnn Prefix'),
      'plDescription' => _('SupAnn Prefix'),
      'plObjectClass' => ['fdSupannPrefix'],
      'plFilter'      => '(objectClass=fdSupannPrefix)',
      'plObjectType'  => [
        'supannPrefix' => [
          'name'         => _('SupAnn Prefix'),
          'description'  => _('SupAnn Prefix'),
          'ou'           => get_ou('SupannPrefixRDN'),
          'icon'         => 'geticon.php?context=applications&icon=supann-entite&size=16',
          'mainAttr'     => 'fdSupannName',
        ],
      ],
      'plSearchAttrs'  => ['description', 'fdSupannName'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo(), TRUE),
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Supann Prefix'),
        'attrs' => [
          new StringAttribute(
            _('Prefix Name'), _('The name of this Supann Prefix'),
            'fdSupannName', TRUE
          ),
          new StringAttribute(
            _('Prefix Label'), _('The label of this Supann Prefix'),
            'fdSupannLabel', TRUE
          ),
          new StringAttribute(
            _('Prefix Description'), _('The description explaining this Supann Prefix'),
            'description', FALSE
          ),
        ],
      ],
      'lists' => [
        'name'  => _('Available Lists'),
        'attrs' => [
          new ObjectsAttribute(
            _('Available Lists'),
            _('Select one or more available Lists for this Prefix'),
            'fdSupannList',
            FALSE,
            [
              'supannActivite',
              'supannCorps',
              'supannDiplome',
              'supannDiscipline',
              'supannEntite',
              'supannEtablissement',
              'supannEtuRegimeInscription',
              'supannRole',
              'supannTypeDiplome'
            ],
          ),
        ]
      ],
      'type' => [
        'name'  => _('Type Lists'),
        'attrs' => [
          new SelectAttribute(
            _('Type Lists'),
            _('Select one type for the Prefix'),
            'fdSupannType',
            TRUE,
            [
              'Aucun',
              'Activite',
              'Corps',
              'Diplome',
              'Discipline',
              'Entite',
              'Etablissement',
              'EtuRegimeInscription',
              'Role',
              'TypeDiplome'
            ],
            NULL,
          ),
        ]
      ],
    ];
  }

  public function compute_dn (): string
  {
    global $config;
    if (!$this->mainTab) {
      throw new FatalError(htmlescape(_('Only main tab can compute dn')));
    }
    if (!isset($this->parent) || !($this->parent instanceof simpleTabs)) {
      throw new FatalError(
        htmlescape(sprintf(
                     _('Could not compute dn: no parent tab class for "%s"'),
                     get_class($this)
                   ))
      );
    }
    $infos = $this->parent->objectInfos();
    if ($infos === FALSE) {
      throw new FatalError(
        htmlescape(sprintf(
                     _('Could not compute dn: could not find objectType info from tab class "%s"'),
                     get_class($this->parent)
                   ))
      );
    }
    $attr = $infos['mainAttr'];
    $ou   = $infos['ou'];

    // Change $this->dn to be in the ou of the type
    $typeValue = $this->attributesAccess['fdSupannType']->getValue();
    $typeRDN   = get_ou('Supann' . $typeValue . 'RDN');

    // Remove SupannListsRDN from typeRDN
    $typeRDN = str_replace(
      get_ou('SupannListsRDN'),
      '',
      $typeRDN
    );

    // Actual dn is fdSupannName=<name>,ou=prefix,<base>
    $ou = str_replace(
      get_ou('SupannPrefixRDN'),
      $typeRDN . get_ou('SupannPrefixRDN'),
      $ou
    );

    if (isset($this->base)) {
      $base = $this->base;
    } else {
      $base = $config->current['BASE'];
    }
    if ($this->is_template) {
      return 'cn=' . ldap_escape_dn($this->_template_cn) . ',ou=templates,' . $ou . $base;
    }

    return $attr . '=' . ldap_escape_dn($this->attributesAccess[$attr]->computeLdapValue()) . ',' . $ou . $base;
  }

  public function prepare_save (): array
  {
    // Get the value from fdSupannTypeObjectclass
    $oc = $this->attributesAccess['fdSupannType']->getValue();

    if ($oc == 'Aucun') {
      return [_('Select a Type for the Supann prefix')];
    }

    return parent::prepare_save();
  }
}
