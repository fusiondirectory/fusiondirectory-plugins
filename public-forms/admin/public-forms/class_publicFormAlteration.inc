<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019 FusionDirectory

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

class publicFormAlteration extends simplePlugin
{
  public static function plInfo (): array
  {
    return array(
      'plShortName'   => _('Altered fields'),
      'plDescription' => _('Alteration on form fields'),
      'plObjectClass' => array(),
      'plObjectType'  => array('publicForm'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return array(
      'main' => array(
        'name'  => _('Altered fields'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Alteration on form fields'),
              'fdPublicFormAlteredAttributes',
              array(
                new StringAttribute(
                  _('Field'), _('LDAP name of a form field'),
                  'fdPublicFormAlteredAttributes_field', TRUE
                ),
                new BooleanAttribute(
                  _('Mandatory'), _('Should this field be mandatory'),
                  'fdPublicFormAlteredAttributes_mandatory', TRUE,
                  FALSE, '',
                  'mandatory', ''
                ),
                new BooleanAttribute(
                  _('Read only'), _('Should this field be read only'),
                  'fdPublicFormAlteredAttributes_readonly', TRUE,
                  FALSE, '',
                  'readonly', ''
                ),
                new BooleanAttribute(
                  _('Imported'), _('Should this field be imported from HTTP headers'),
                  'fdPublicFormAlteredAttributes_imported', TRUE,
                  FALSE, '',
                  'imported', ''
                ),
                new BooleanAttribute(
                  _('Hidden'), _('Should this field be hidden away'),
                  'fdPublicFormAlteredAttributes_hidden', TRUE,
                  FALSE, '',
                  'hidden', ''
                ),
              ),
              '',
              ''
            ),
            // no order
            FALSE,
            // default value
            array('base||||hidden'),
            // edit enabled
            TRUE
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    $this->attributesAccess['fdPublicFormAlteredAttributes']->setHeaders(
      array(
        _('Field'),
        _('Mandatory'),
        _('Read only'),
        _('Imported'),
        _('Hidden'),
      )
    );
  }
}
