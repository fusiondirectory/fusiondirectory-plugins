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

class sinapsRequest
{
  protected $data;

  public function __construct($xml)
  {
    $this->data = new SimpleXMLElement($xml);
  }

  public function codeDomaine()
  {
    return $this->data->domaine['code'];
  }

  public function codeOperation()
  {
    return $this->data->domaine->donnees->operation['codeOperation'];
  }

  public function operationVersion()
  {
    return $this->data->domaine->donnees->operation['version'];
  }

  public function identifiantTransaction()
  {
    return $this->data->domaine->identifiantTransaction;
  }

  public function getOperationStructure()
  {
    return $this->data->domaine->donnees->operation->structure;
  }

  public function getOperationPersonne()
  {
    return $this->data->domaine->donnees->operation->personne;
  }

  public function acquittementFonctionnel($responseCode = 200, $codeAcquittement = 0, $message = '', $identifiantObjApp = NULL)
  {
    $xml = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>
    <Acquittement xmlns:acq="http://referentiels.SINAPS.amue.fr/acquittementFonctionnel">
    </Acquittement>');
    $xml->addChild('ResponseCode', $responseCode);
    $xml->addChild('messageAcquittement', $message);
    $xml->addChild('codeAcquittement', $codeAcquittement);
    if ($identifiantObjApp) {
      $xml->addChild('identifiantObjApp', $identifiantObjApp);
    }

    return $xml->asXML();
  }

  public function acquittementTechnique($responseCode = 200, $message = '')
  {
    $xml = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>
    <WebServiceHTTPResponse xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="acquittementTechnique.xsd">
    </WebServiceHTTPResponse>');
    $xml->addChild('IdentifiantTransaction', $this->identifiantTransaction());
    $xml->addChild('ResponseCode', $responseCode);
    $xml->addChild('ResponseMessage', $message);

    return $xml->asXML();
  }

  public function getSupannEntiteValues($identifiantApplication, $uuidPrefix)
  {
    $structure = $this->getOperationStructure();

    $mapping = array(
      'libelle20'         => array('entite','ou'),
      'descriptifLong'    => array('entite','description'),
      'codeStructure'     => array('entite','supannCodeEntite'),
      'codeSousType'      => array('entite','supannTypeEntite'),
      'dateDebutValidite' => array('supannStructureExt','fdSupannStartDate'),
      'dateFinValidite'   => array('supannStructureExt','fdSupannEndDate'),
    );
    $values = array(
      'entite' => array(
        'supannRefId'               => array(),
        'telephoneNumber'           => '',
        'facsimileTelephoneNumber'  => '',
        'postalAddress'             => '',
      ),
    );

    $uuid = '';

    if ($structure->referenceCroisee->identifiantsExternes != NULL) {
      foreach ($structure->referenceCroisee->identifiantsExternes as $ref) {
        if ($ref->identifiantApplication == $identifiantApplication) {
          $uuid = $ref->identifiantExterne;
        } else {
          $values['entite']['supannRefId'][] = '{'.$ref->identifiantApplication.'}'.$ref->identifiantExterne;
        }
      }
    }

    if (empty($uuid)) {
      $uuid = static::genUUID();
    }

    array_unshift($values['entite']['supannRefId'], '{'.$uuidPrefix.'}'.$uuid);

    foreach ($mapping as $sinapsAttr => list($fdTab,$fdAttr)) {
      if (isset($structure->$sinapsAttr)) {
        $values[$fdTab][$fdAttr] = (string)$structure->$sinapsAttr;
      }
    }

    $now = new DateTime();
    if ($structure->methodesDeContact->methodeDeContact != NULL) {
      foreach ($structure->methodesDeContact->methodeDeContact as $method) {
        if ($method->dateDebutValidite) {
          $dateDebutValidite = new DateTime($method->dateDebutValidite);
          if ($dateDebutValidite >= $now) {
            // Method is not valid yet, skip it
            continue;
          }
        }
        if ($method->dateFinValidite) {
          $dateFinValidite = new DateTime($method->dateFinValidite);
          if ($dateFinValidite < $now) {
            // Method is not valid anymore, skip it
            continue;
          }
        }
        switch ((string)$method->codeTypeMethodeContact) {
          case 'TEL':
            $values['entite']['telephoneNumber'] = (string)$method->valeur;
            break;
          case 'FAX':
            $values['entite']['facsimileTelephoneNumber'] = (string)$method->valeur;
            break;
          case 'ADR':
            if ((string)$method->adresse->temoinAdressePrincipale != 'true') {
              trigger_error('Ignore adresse non principale');
              continue;
            }
            $adresse = $this->convertAddress($method->adresse);
            if ($adresse === FALSE) {
              trigger_error('Ignore adresse non FR non ETR');
              continue;
            }
            $values['entite']['postalAddress'] = $adresse;
            break;
          default:
            trigger_error('ignoring codeTypeMethodeContact '.$method->codeTypeMethodeContact);
        }
      }
    }

    if ($structure->liensStructure->lienStructure != NULL) {
      foreach ($structure->liensStructure->lienStructure as $lienStructure) {
        if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
          //TODO: vérifier date de début/fin
          $mereUUID = $this->getUUIDFromReferenceCroisee($identifiantApplication, $lienStructure->referenceCroisee);
          if (!empty($mereUUID)) {
            // TODO: warning si plusieurs?
            $values['entite']['supannCodeEntiteParent'] = '{LDAPUUID}'.$mereUUID;
          }
        }
      }
    }

    return $values;
  }

  public function getUserValues($identifiantApplication, $uuidPrefix)
  {
    $personne = $this->getOperationPersonne();
    $mapping = array(
      'civilite'        => array('supannAccount','supannCivilite'),
      'nomUsage'        => array('user','sn'),
      'descriptifLong'  => array('user','description'),
      'dateNaissance'   => array('personalInfo','dateOfBirth'),
      'sexe'            => array('personalInfo','gender'),
    );
    $values = array(
      'user'          => array(),
      'supannAccount' => array(
        'supannRefId' => array(),
      ),
      'personalInfo'  => array(),
    );

    $uuid = '';

    if ($structure->referenceCroisee->identifiantsExternes != NULL) {
      foreach ($personne->referenceCroisee->identifiantsExternes as $ref) {
        if ($ref->identifiantApplication == $identifiantApplication) {
          $uuid = $ref->identifiantExterne;
        } else {
          $values['supannAccount']['supannRefId'][] = '{'.$ref->identifiantApplication.'}'.$ref->identifiantExterne;
        }
      }
    }

    if (empty($uuid)) {
      $uuid = static::genUUID();
    }

    array_unshift($values['supannAccount']['supannRefId'], '{'.$uuidPrefix.'}'.$uuid);

    foreach ($mapping as $sinapsAttr => list($fdTab,$fdAttr)) {
      if (isset($personne->$sinapsAttr)) {
        $values[$fdTab][$fdAttr] = (string)$personne->$sinapsAttr;
      }
    }

    if (isset($personne->prenoms)) {
      //TODO stocker les autres prénoms?
      $values['user']['givenName'] = preg_replace('/;.*$/', '', $personne->prenoms);
    }

    //TODO: methodeContact ou methodeDeContact?
    foreach ($personne->methodesDeContact->methodeContact as $method) {
      switch ((string)$method->typeMethodeContact) {
        case 'TEL':
          $values['user']['telephoneNumber'] = (string)$method->valeur;
          //TODO supannAutreMail
          //TODO supannAutreTelephone
          break;
        case 'FAX':
          $values['user']['facsimileTelephoneNumber'] = (string)$method->valeur;
          break;
        case 'MAILPERSO':
          $values['supannAccount']['supannMailPerso'] = (string)$method->valeur;
          break;
        case 'ADR':
          if ((string)$method->adresse->temoinAdressePrincipale != 'true') {
            trigger_error('Ignore adresse non principale');
            continue;
          }
          $adresse = $this->convertAddress($method->adresse);
          if ($adresse === FALSE) {
            trigger_error('Ignore adresse non FR non ETR');
            continue;
          }
          if ($method->adresse->codeTypeAdresse == 'ADRPERSO') {
            $values['user']['homePostalAddress'] = $adresse;
          } else {
            $values['user']['postalAddress'] = $adresse;
          }
          break;
        default:
          trigger_error('ignoring codeTypeMethodeContact '.$method->typeMethodeContact);
      }
    }

    //~ foreach ($personne->liensStructure->lienStructure as $lienStructure) {
      //~ if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
        //~ //TODO: vérifier date de début, peut-être prendre identifiantExterne
        //~ $values['user']['supannCodeEntiteParent'] = (string)$lienStructure->codeStructureMere;
      //~ }
    //~ }

    return $values;
  }

  protected function convertAddress($address)
  {
    $adresse = '';
    if (!empty($address->adresseFR)) {
      if (!empty($address->adresseFR->batiment)) {
        $adresse .= $address->adresseFR->batiment;
        if (!empty($address->adresseFR->etage)) {
          $adresse .= ', '.$address->adresseFR->etage;
        }
        $adresse .= "\n";
      }
      $adresse .= $address->adresseFR->numeroVoie;
      if (!empty($address->adresseFR->codeBTQC)) {
        $adresse .= ' '.$address->adresseFR->codeBTQC;
      }
      $adresse .= ' '.$address->adresseFR->typeVoie;
      $adresse .= ' '.$address->adresseFR->nomVoie;
      $adresse .= "\n";
      if (!empty($address->adresseFR->complement)) {
        $adresse .= $address->adresseFR->complement."\n";
      }
      if (!empty($address->adresseFR->lieuDit)) {
        $adresse .= $address->adresseFR->lieuDit."\n";
      }
      $adresse .= $address->adresseFR->codePostal;
      $adresse .= ' '.$address->adresseFR->BD;
      $adresse .= "\n";
      $adresse .= "FRANCE\n";
    } elseif (!empty($address->adresseEtr)) {
      if (!empty($address->adresseEtr->batiment)) {
        $adresse .= $address->adresseEtr->batiment;
        if (!empty($address->adresseEtr->etage)) {
          $adresse .= ', '.$address->adresseEtr->etage;
        }
        $adresse .= "\n";
      }
      $adresse .= $address->adresseEtr->numeroVoie;
      $adresse .= ' '.$address->adresseEtr->nomVoie;
      $adresse .= "\n";
      if (!empty($address->adresseEtr->complement)) {
        $adresse .= $address->adresseEtr->complement."\n";
      }
      $adresse .= $address->adresseEtr->codePostal;
      $adresse .= ' '.$address->adresseEtr->ville;
      $adresse .= "\n";
      $adresse .= $address->adresseEtr->etat."\n";
      $adresse .= $address->adresseEtr->pays."\n";
    } else {
      $adresse = FALSE;
    }

    return $adresse;
  }

  protected function getUUIDFromReferenceCroisee($identifiantApplication, $referenceCroisee)
  {
    foreach ($referenceCroisee->identifiantsExternes as $ref) {
      if ($ref->identifiantApplication == $identifiantApplication) {
        return $ref->identifiantExterne;
      }
    }

    return '';
  }

  protected static function genUUID()
  {
    //UUID v4
    return sprintf(
      '%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
      // 32 bits for "time_low"
      mt_rand(0, 0xffff), mt_rand(0, 0xffff),
      // 16 bits for "time_mid"
      mt_rand(0, 0xffff),
      // 16 bits for "time_hi_and_version",
      // four most significant bits holds version number 4
      mt_rand(0, 0x0fff) | 0x4000,
      // 16 bits, 8 bits for "clk_seq_hi_res",
      // 8 bits for "clk_seq_low",
      // two most significant bits holds zero and one for variant DCE1.1
      mt_rand(0, 0x3fff) | 0x8000,
      // 48 bits for "node"
      mt_rand(0, 0xffff), mt_rand(0, 0xffff), mt_rand(0, 0xffff)
    );
  }
}
