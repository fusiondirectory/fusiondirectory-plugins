<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2026  FusionDirectory

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

class supann
{
  static $choices_for = [];

  static function get_prefixed_choices_for (string $shortname, $processLabel = 'supann::truncate_label')
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd(get_ou('SupannPrefix').get_ou('SupannListsRDN').$config->current['BASE']);
    $ldap->search("(&(objectClass=fdSupannPrefix)(objectClass=fdSupann" . $shortname . "))", [
      'fdSupannName',
      'fdSupannList'
    ]);

    $res = [];
    while ($attrs = $ldap->fetch()) {
      $prefix        = $attrs['fdSupannName'][0];
      $choicesCodes  = [];
      $choicesLabels = [];

      // Remove count
      unset($attrs['fdSupannList']['count']);

      // Get choices
      $choices = self::get_choices_for($shortname, $attrs['fdSupannList'], $processLabel);

      /* Format is 
         PREFIX => [
           [
             CODE1,
             CODE2,
             ...
           ],
           [
             LABEL1,
             LABEL2,
             ...
           ]
         ]
      */
      $res[$prefix] = $choices;
    }

    ksort($res);
    return $res;
  }

  static function get_choices_for (string $shortname, array|string $dns = '', $processLabel = 'supann::truncate_label')
  {
    global $config;

    $ldap = $config->get_ldap_link();

    $choicesCodes  = [];
    $choicesLabels = [];

    foreach ($dns as $dn) {
      $ldap->cd($dn);
      $ldap->search('(objectClass=fdSupann' . $shortname . ')', [
        'fdSupannName',
        'fdSupannLabel'
      ]);

      while ($dnAttrs = $ldap->fetch()) {
        // fdSupannName contain the prefix but the functions doesn't want it
        $choicesCodes[]  = explode("}", $dnAttrs['fdSupannName'][0])[1];
        $choicesLabels[] = $processLabel($dnAttrs['fdSupannLabel'][0]);
      }
    }

    array_multisort($choicesLabels, $choicesCodes);
    return [$choicesCodes, $choicesLabels];
  }

  /*! \brief Returns the $len first chars and "…" after if text is longer */
  static function truncate_label ($str, $len = 50)
  {
    if (mb_strlen($str) > $len) {
      return mb_substr($str, 0, $len).'…';
    } else {
      return $str;
    }
  }

  /*! \brief Return the informations for a specific supannCodePopulation */
  static function get_supannCodePopulation_info (string $codePopulation) : array
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd(get_ou('SupannObjectsRDN').$config->current['BASE']);
    $ldap->search("(&(objectClass=fdSupannPopulationCode)(fdSupannPopulationCodeName=$codePopulation))", [
      'fdSupannPopulationCodeWeight',
      'fdSupannPopulationEduPersonAffiliation',
      'fdSupannPopulationEduPersonPrimaryAffiliation'
    ]);
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['fdSupannPopulationEduPersonAffiliation'])) {
        $result['eduPersonAffiliation'] = $attrs['fdSupannPopulationEduPersonAffiliation'];
      }

      if (isset($attrs['fdSupannPopulationEduPersonPrimaryAffiliation'])) {
        $result['eduPersonPrimaryAffiliation'] = $attrs['fdSupannPopulationEduPersonPrimaryAffiliation'];
      }

      if (isset($attrs['fdSupannPopulationCodeWeight'])) {
        $result['weight'] = $attrs['fdSupannPopulationCodeWeight'];
      }
    }

    if (isset($result)) {
      LDAP::unsetCountKeys($result);
    } else {
      $result = [];
    }

    return $result;
  }
}
