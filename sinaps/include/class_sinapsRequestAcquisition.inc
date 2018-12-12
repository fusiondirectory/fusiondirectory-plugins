<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018 FusionDirectory

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

class sinapsRequestAcquisition
{
  protected $data;
  protected $identifiantTransaction;

  public static $attributes = array(
    'supannRefId'                       => '*',
    'supannEntiteAffectationPrincipale' => 1,
  );

  public function __construct()
  {
    $this->identifiantTransaction = sinapsRequest::genUUID();
    $xml = '<?xml version="1.0" encoding="UTF-8" ?>
<socle:root xmlns:socle="http://socle.sinaps.amue.fr/V1_1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:sinaps="http://common.nomenclature.sinaps.amue.fr/V1_1" xmlns:adr="http://referentiels.sinaps.amue.fr/adresseContact/V1_0" xmlns:ie="http://xml.insee.fr/schema" xmlns:prs="http://referentiels.sinaps.amue.fr/personne/V1_0">
  <domaine code="PERSONNE">
    <identifiantTransaction>'.$this->identifiantTransaction.'</identifiantTransaction>
    <donnees>
      <operation codeOperation="ACQUISITION" version="1.0">
       <personne>
        </personne>
      </operation>
    </donnees>
  </domaine>
</socle:root>';
    $this->data = new SimpleXMLElement($xml);
  }

  public function fill(array $user, string $uuidPrefix, string $typeExterne, array $methodeContactMapping, $codeEntiteToldapUuidCallback)
  {
    $personne = $this->data->domaine->donnees->operation->personne;

    /* identifiantExternePersonne */
    if (isset($user['supannRefId'])) {
      foreach ($user['supannRefId'] as $supannRefId) {
        if (preg_match('/^{'.preg_quote($uuidPrefix).'}(.+)$/', $supannRefId, $m)) {
          $uuid = $m[1];
        }
      }
    }
    if (!isset($uuid)) {
      throw new FusionDirectoryException('Cound not find uuid in supannRefId');
    }
    $personne->identifiantExternePersonne = $uuid;

    /* methodesDeContact */
    $i = 0;
    foreach ($methodeContactMapping as $ldapField => $sinapsType) {
      if (isset($user[$ldapField])) {
        $personne->methodesDeContact->methodeContact[$i]->typeMethodeContact  = $sinapsType;
        $personne->methodesDeContact->methodeContact[$i]->valeur              = $user[$ldapField];
        $i++;
      }
    }

    /* rattachements */
    if (isset($user['supannEntiteAffectationPrincipale'])) {
      $personne->rattachements->rattachement->identifiantStructureDeRattachement  = $codeEntiteToldapUuidCallback($user['supannEntiteAffectationPrincipale']);
      $personne->rattachements->rattachement->roles->role->typeRole               = 'EXT';
      $personne->rattachements->rattachement->roles->role->externe->typeExterne   = $typeExterne;
    }
  }

  public function getXML()
  {
    return $this->data->asXML();
  }
}
