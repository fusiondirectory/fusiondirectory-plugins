<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

class supannStructureExt extends simplePlugin
{
  var $objectclasses = array('fdSupannExt');

  var $mainTab = TRUE;

  /* Return plugin information */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('SUPANN-Ext'),
      'plDescription' => _('SUPANN Non-Official Extension'),
      'plObjectType'  => array('entite', 'etablissement'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'validity' => array(
        'name'  => _('Validity'),
        'attrs' => array(
          new GeneralizedTimeDateAttribute(
            _('Start date'), _('The date from which this structure will be valid'),
            'fdSupannStartDate', FALSE,
            ''
          ),
          new GeneralizedTimeDateAttribute(
            _('End date'), _('The date after which this structure will not be valid anymore'),
            'fdSupannEndDate', FALSE,
            ''
          ),
        )
      ),
      'relation' => array(
        'name'  => _('Relations'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          /* These attributes are handled by the SupannOrderedArrayAttribute */
          new HiddenAttribute('fdSupannRelationEntiteType'),
          new HiddenAttribute('fdSupannRelationEntiteRole'),
          new HiddenAttribute('fdSupannRelationEntiteCode'),
          new SupannOrderedArrayAttribute(
            new SupannCompositeAttribute(
              _('fdSupannRelationEntite'),
              'fdSupannRelationEntite',
              array(
                new SupannPrefixedSelectAttribute(
                  _('Role'), _('Role'),
                  'fdSupannRelationEntiteRole_role', TRUE,
                  'relationrole'
                ),
                new SupannPrefixedSelectAttribute(
                  _('Type'), _('Type'),
                  'fdSupannRelationEntiteType_type', TRUE,
                  'relationtype'
                ),
                new SelectAttribute(
                  _('Entity'), _('Entity'),
                  'fdSupannRelationEntiteCode_code', FALSE
                ),
              ),
              '',
              // no label
              ''
            ),
            // no order
            FALSE,
            array(),
            // no edit button
            FALSE
          )
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannEntite)', array('supannCodeEntite', 'ou', 'o','supannEtablissement'));

    $code_entites   = array();
    $label_entites  = array();
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        $code_entites[] = $attrs['supannCodeEntite'][0];
        if (isset($attrs['ou'][0])) {
          $label_entites[] = $attrs['ou'][0]." (".$attrs['supannCodeEntite'][0].")";
        } else {
          $label_entites[] = $attrs['o'][0]." (".$attrs['supannCodeEntite'][0].")";
        }
      }
    }
    array_multisort($label_entites,         $code_entites);

    $this->attributesAccess['fdSupannRelationEntite']->attribute->attributes[2]->setChoices($code_entites, $label_entites);
  }

  protected function prepare_save ()
  {
    $this->attributesAccess['fdSupannRelationEntite']->supannPrepareSave();
    return parent::prepare_save();
  }
}

?>
