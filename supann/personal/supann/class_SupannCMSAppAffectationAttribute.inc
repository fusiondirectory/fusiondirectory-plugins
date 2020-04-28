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

class SupannCMSAppAffectationAttribute extends SupannCompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = '')
  {
    global $config;
    list($types, $typeLabels)     = supann::get_choices_for('cmstype');
    $attributes = [
      new SelectAttribute(
        _('Type'), _('supannCMSType - Card type'),
        'supannCMSType_type_'.$ldapName, TRUE,
        $types, '', $typeLabels
      ),
      new SelectAttribute(
        _('Source'), _('Card managing system and establishment'),
        'supannCMSSource_source_'.$ldapName, TRUE,
        $config->get_cfg_value('supannCMSSources', [])
      ),
      new SelectAttribute(
        _('Domain'), _('Application domain'),
        'none_domaine_'.$ldapName, TRUE,
        array_column($this->getConfigAppDomains(), 1)
      ),
      new StringAttribute(
        _('Id'), _('Application id'),
        'none_id_'.$ldapName, TRUE
      ),
      new BooleanAttribute(
        _('Valid'), _('Application validity'),
        'none_valide_'.$ldapName, TRUE,
        TRUE, '',
        'vrai', 'faux'
      ),
      new GeneralizedTimeDateAttribute(
        _('End date'), _('End of validity date, if any'),
        'supannCMSDateFin_datefin_'.$ldapName, FALSE,
        ''
      ),
    ];
    parent::__construct($description, $ldapName, $attributes, $acl, $label);
  }

  protected function isValid (): bool
  {
    return ($this->attributes[4]->getValue() === TRUE);
  }

  protected function getConfigAppDomains (): array
  {
    global $config;
    return array_map(
      function ($supannCMSAppDomain) {
        return explode(';', $supannCMSAppDomain, 2);
      },
      $config->get_cfg_value('supannCMSAppDomains', [])
    );
  }

  function supannGetValues (&$values)
  {
    foreach ($this->attributes as &$attribute) {
      list($attrname, ) = explode('_', $attribute->getLdapName());
      if ($attrname == 'none') {
        continue;
      }
      $value = $attribute->getValue();
      if (!isset($values[$attrname])) {
        $values[$attrname] = [];
      }
      if ($value == '') {
        continue;
      }
      if ($this->isValid()) {
        $values[$attrname][$value] = $value;
      }
    }
    unset($attribute);
    if ($this->isValid()) {
      $domains  = array_column($this->getConfigAppDomains(), 0, 1);
      $domain   = $this->attributes[2]->getValue();
      $id       = $this->attributes[3]->getValue();
      $supannCMSAppIdDomaine  = $id.'@'.$domain;
      $supannCMSAppId         = $domains[$domain].';'.$id;
      $values['supannCMSAppIdDomaine'][$supannCMSAppIdDomaine] = $supannCMSAppIdDomaine;
      $values['supannCMSAppId'][$supannCMSAppId] = $supannCMSAppId;
    } else {
      if (!isset($values['supannCMSAppIdDomaine'])) {
        $values['supannCMSAppIdDomaine'] = [];
      }
      if (!isset($values['supannCMSAppId'])) {
        $values['supannCMSAppId'] = [];
      }
    }
  }
}
