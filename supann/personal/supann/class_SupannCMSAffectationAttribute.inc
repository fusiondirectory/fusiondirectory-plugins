<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2020 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class SupannCMSAffectationAttribute extends SupannCompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = '')
  {
    list($types, $typeLabels)     = supann::get_choices_for('cmstype');
    list($formats, $formatLabels) = supann::get_choices_for('cmsformat');
    $attributes = [
      new SelectAttribute(
        _('Type'), _('supannCMSType - Card type'),
        'supannCMSType_type', TRUE,
        $types, '', $typeLabels
      ),
      new SelectAttribute(
        _('Format'), _('Card format'),
        'none_format', TRUE,
        $formats, '', $formatLabels
      ),
      new StringAttribute(
        _('Id'), _('Card id'),
        'none_id', TRUE
      ),
      new BooleanAttribute(
        _('Valid'), _('Card validity'),
        'none_valid', TRUE,
        TRUE
      ),
      new SelectAttribute(
        _('Source'), _('Card managing system and establishment'),
        'supannCMSSource_source', FALSE
      ),
      new GeneralizedTimeDateAttribute(
        _('End date'), _('End of validity date, if any'),
        'supannCMSDateFin_datefin', FALSE,
        ''
      ),
    ];
    parent::__construct($description, $ldapName, $attributes, $acl, $label);
  }

  function supannGetValues (&$values)
  {
    foreach ($this->attributes as &$attribute) {
      $shortname = preg_replace('/^([^_]+)_.*$/', '\\1', $attribute->getLdapName());
      if ($shortname == 'none') {
        continue;
      }
      $value = $attribute->getValue();
      if (!isset($values[$shortname])) {
        $values[$shortname] = [];
      }
      if ($value == '') {
        continue;
      }
      if ($this->attributes[3]->getValue() === TRUE) {
        /* Card is valid */
        $values[$shortname][$value] = $value;
      }
    }
    unset($attribute);
    if ($this->attributes[3]->getValue() === TRUE) {
      /* Card is valid */
      $format = $this->attributes[1]->getValue();
      $id     = $this->attributes[2]->getValue();
      $supannCMSIdEtiquette = '{'.$format.'}'.$id;
      $supannCMSId          = 'x-'.strtolower(str_replace(':', '-', $format)).';'.$id;
      $values['supannCMSIdEtiquette'][$supannCMSIdEtiquette] = $supannCMSIdEtiquette;
      $values['supannCMSId'][$supannCMSId] = $supannCMSId;
    } else {
      if (!isset($values['supannCMSIdEtiquette'])) {
        $values['supannCMSIdEtiquette'] = [];
      }
      if (!isset($values['supannCMSId'])) {
        $values['supannCMSId'] = [];
      }
    }
  }
}
