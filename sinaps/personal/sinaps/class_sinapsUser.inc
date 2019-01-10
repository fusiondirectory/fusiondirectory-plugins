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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class sinapsUser extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Sinaps'),
      'plTitle'         => _('Sinaps'),
      'plDescription'   => _('Used to send acquisition request for users'),
      'plFilter'        => '(objectClass=*)',
      'plSelfModify'    => FALSE,
      'plObjectClass'   => array(),
      'plObjectType'    => array('user'),
      //~ 'plIcon'          => '',
      //~ 'plSmallIcon'     => '',

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Sinaps'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DisplayAttribute (
            '', '',
            'sinaps_explanation', FALSE,
            _('This tab takes care of sending Acquisition event to Sinaps whenever a user is modified.')
          ),
        )
      )
    );
  }

  function codeEntiteToldapUuidCallback($codeEntite)
  {
    global $config;

    /* Translate code entite to UUID refs
     * This is done here to make sure sinapsRequestAcquisition does not perform LDAP searches by itself
     * */
    $entites = objects::ls(
      'entite',
      array('supannRefId' => '*', 'supannTypeEntite' => 1, 'dn' => 'raw'),
      NULL,
      '(supannCodeEntite='.$codeEntite.')'
    );
    if (empty($entites)) {
      $error = 'Could not find entity '.$codeEntite;
    } elseif (count($entites) > 1) {
      $error = 'Multiple entite matches codeEntite '.$codeEntite;
    } else {
      $entite = reset($entites);
      foreach ($entite['supannRefId'] as $supannRefId) {
        if (preg_match('/^{'.preg_quote($config->get_cfg_value('SinapsUuidPrefix', 'LDAPUUID'), '/').'}(.+)$/', $supannRefId, $m)) {
          return $m[1];
        }
      }
      $error = 'Could not find any UUID for '.$entite['dn'];
    }

    throw new FusionDirectoryException($error);
  }

  protected function prepare_save ()
  {
    global $config;

    /* We have to work in prepare_save as the rest won’t get called since there is nothing to save */

    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

    if ($config->get_cfg_value('SinapsEnabled', 'FALSE') == 'FALSE') {
      return array();
    }
    if ($config->get_cfg_value('SinapsAcquisitionURL', '') == '') {
      return array('Sinaps acquisition URL is not filled in FusionDirectory configuration');
    }

    $contractEnded = FALSE;
    if (isset($this->parent->by_object['personalInfo']) && ($this->parent->by_object['personalInfo']->attributesAccess['fdContractEndDate']->getValue() != '')) {
      $fdContractEndDate  = $this->parent->by_object['personalInfo']->attributesAccess['fdContractEndDate']->getDateValue();
      $tomorrow           = new DateTime('tomorrow');
      if ($fdContractEndDate < $tomorrow) {
        if ($this->parent->by_object['personalInfo']->attributesAccess['fdContractEndDate']->hasChanged()) {
          $contractEnded = TRUE;
        } else {
          /* Do not sync already deleted users */
          return;
        }
      }
    }

    $attributes = sinapsRequestAcquisition::$attributes;

    $mapping = array();
    foreach ($config->get_cfg_value('SinapsAcquisitionContactMethodMap', array()) as $field) {
      list($ldapField, $sinapsType) = explode('|', $field, 2);

      $mapping[$ldapField]    = $sinapsType;
      $attributes[$ldapField] = 1;
    }

    $attributes['dn'] = 'raw';

    $users  = objects::ls('user', $attributes, $this->dn, '', FALSE, 'base');
    $user   = reset($users);

    if (!is_array($user)) {
      return;
    }

    try {
      $request = new sinapsRequestAcquisition();
      $request->fill($user, $config->get_cfg_value('SinapsUuidPrefix', 'LDAPUUID'), $config->get_cfg_value('SinapsAcquisitionTypeExterne', 'FD'), $mapping, array($this, 'codeEntiteToldapUuidCallback'), $contractEnded);
      $xml = $request->getXml();
      $date = date('Y-m-d_H-i-s');
      sinapsDiffusionHandlerJob::dumpFile($date.'_acquittement.xml', $xml);

      if ($config->get_cfg_value('SinapsDryRun') != 'TRUE') {
        $answer = sinapsDiffusionHandlerJob::sendPostRequest($config->get_cfg_value('SinapsAcquisitionURL'), $config->get_cfg_value('SinapsLogin', 'fusiondirectory'), $config->get_cfg_value('SinapsPassword'), $xml);
        sinapsDiffusionHandlerJob::dumpFile($date.'_acquittement-answer.xml', $answer);
      }
    } catch (FusionDirectoryException $e) {
    }
  }
}
