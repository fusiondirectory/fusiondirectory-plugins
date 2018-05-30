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
    return '<?xml version="1.0" encoding="UTF-8"?>
<socle:root xmlns:socle="http://socle.sinaps.amue.fr/V1_1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:acq="http://referentiels.SINAPS.amue.fr/acquittementFonctionnel">
<domaine code="ACQUITTEMENT">
  <donnees>
    <operation codeOperation="CREATION" version="1.0">
      <acq:Acquittement>
        <identifiantTransaction>'.$this->identifiantTransaction().'</identifiantTransaction>
        <ResponseCode>'.$responseCode.'</ResponseCode>
        <codeAcquittement>'.$codeAcquittement.'</codeAcquittement>
        <messageAcquittement>'.$message.'</messageAcquittement>
        <identifiantObjApp>'.$identifiantObjApp.'</identifiantObjApp>
      </acq:Acquittement>
    </operation>
  </donnees>
</domaine>
</socle:root>';
  }

  public function acquittementTechnique($responseCode = 200, $message = '')
  {
    return '<?xml version="1.0" encoding="UTF-8"?>
<WebServiceHTTPResponse xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="acquittementTechnique.xsd">
  <IdentifiantTransaction>'.$this->identifiantTransaction().'</IdentifiantTransaction>
  <ResponseCode>'.$responseCode.'</ResponseCode>
  <ResponseMessage>'.$message.'</ResponseMessage>
</WebServiceHTTPResponse>';
  }

  public function getSupannEntiteValues()
  {
    $structure = $this->getOperationStructure();
    $mapping = array(
      'libelle20'       => 'ou',
      'descriptifLong'  => 'description',
      'codeStructure'   => 'supannCodeEntite',
      'codeNature'      => 'supannTypeEntite',
    );
    $values = array(
      'entite' => array(),
    );

    foreach ($mapping as $sinapsAttr => $fdAttr) {
      if (isset($structure->$sinapsAttr)) {
        $values['entite'][$fdAttr] = (string)$structure->$sinapsAttr;
      }
    }

    foreach ($structure->methodesDeContact->methodeDeContact as $method) {
      switch ((string)$method->codeTypeMethodeContact) {
        case 'TEL':
          $values['entite']['telephoneNumber'] = (string)$method->valeur;
          break;
        case 'FAX':
          $values['entite']['facsimileTelephoneNumber'] = (string)$method->valeur;
          break;
        case 'ADR':
          if ((string)$method->adresse->temoinAdressePrincipale != 'true') {
            echo 'Ignore adresse non principale'."\n";
            continue;
          }
          if (isset($method->adresse->adresseFR)) {
            $adresse = '';
            if (isset($method->adresse->adresseFR->batiment)) {
              $adresse .= $method->adresse->adresseFR->batiment;
              if (isset($method->adresse->adresseFR->etage)) {
                $adresse .= ', '.$method->adresse->adresseFR->etage;
              }
              $adresse .= "\n";
            }
            $adresse .= $method->adresse->adresseFR->numeroVoie;
            if (isset($method->adresse->adresseFR->codeBTQC)) {
              $adresse .= ' '.$method->adresse->adresseFR->codeBTQC;
            }
            $adresse .= ' '.$method->adresse->adresseFR->typeVoie;
            $adresse .= ' '.$method->adresse->adresseFR->nomVoie;
            $adresse .= "\n";
            if (isset($method->adresse->adresseFR->complement)) {
              $adresse .= $method->adresse->adresseFR->complement."\n";
            }
            if (isset($method->adresse->adresseFR->lieuDit)) {
              $adresse .= $method->adresse->adresseFR->lieuDit."\n";
            }
            $adresse .= $method->adresse->adresseFR->codePostal;
            $adresse .= ' '.$method->adresse->adresseFR->BD;
            $adresse .= "\n";
            $adresse .= "FRANCE\n";
          } elseif (isset($method->adresse->adresseETR)) {
            $adresse = '';
            if (isset($method->adresse->adresseETR->batiment)) {
              $adresse .= $method->adresse->adresseETR->batiment;
              if (isset($method->adresse->adresseETR->etage)) {
                $adresse .= ', '.$method->adresse->adresseETR->etage;
              }
              $adresse .= "\n";
            }
            $adresse .= $method->adresse->adresseETR->numeroVoie;
            $adresse .= ' '.$method->adresse->adresseETR->nomVoie;
            $adresse .= "\n";
            if (isset($method->adresse->adresseETR->complement)) {
              $adresse .= $method->adresse->adresseETR->complement."\n";
            }
            $adresse .= $method->adresse->adresseETR->codePostal;
            $adresse .= ' '.$method->adresse->adresseETR->ville;
            $adresse .= "\n";
            $adresse .= $method->adresse->adresseETR->etat."\n";
            $adresse .= $method->adresse->adresseETR->pays."\n";
          } else {
            echo 'Ignore adresse non FR non ETR'."\n";
            continue;
          }
          $values['entite']['postalAddress'] = $adresse;
          break;
        default:
          echo 'ignoring codeTypeMethodeContact '.$method->codeTypeMethodeContact."\n";
      }
    }

    foreach ($structure->liensStructure->lienStructure as $lienStructure) {
      if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
        //TODO: vérifier date de début, peut-être prendre identifiantExterne
        $values['entite']['supannCodeEntiteParent'] = (string)$lienStructure->codeStructureMere;
      }
    }

    $uuid = static::genUUID();
    $values['entite']['supannRefId'] = array(
      '{PASSPORT}'.$uuid,
    );

    return $values;
  }

  public function getUserValues()
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
      'supannAccount' => array(),
      'personalInfo'  => array(),
    );

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
            echo 'Ignore adresse non principale'."\n";
            continue;
          }
          $adresse = $this->convertAddress($method->adresse);
          if ($adresse === FALSE) {
            echo 'Ignore adresse non FR non ETR'."\n";
            continue;
          }
          if ($method->adresse->codeTypeAdresse == 'ADRPERSO') {
            $values['user']['homePostalAddress'] = $adresse;
          } else {
            $values['user']['postalAddress'] = $adresse;
          }
          break;
        default:
          echo 'ignoring codeTypeMethodeContact '.$method->typeMethodeContact."\n";
      }
    }

    //~ foreach ($personne->liensStructure->lienStructure as $lienStructure) {
      //~ if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
        //~ //TODO: vérifier date de début, peut-être prendre identifiantExterne
        //~ $values['user']['supannCodeEntiteParent'] = (string)$lienStructure->codeStructureMere;
      //~ }
    //~ }

    $uuid = static::genUUID();
    $values['supannAccount']['supannRefId'] = array(
      '{PASSPORT}'.$uuid,
    );

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
    } elseif (!empty($address->adresseETR)) {
      if (!empty($address->adresseETR->batiment)) {
        $adresse .= $address->adresseETR->batiment;
        if (!empty($address->adresseETR->etage)) {
          $adresse .= ', '.$address->adresseETR->etage;
        }
        $adresse .= "\n";
      }
      $adresse .= $address->adresseETR->numeroVoie;
      $adresse .= ' '.$address->adresseETR->nomVoie;
      $adresse .= "\n";
      if (!empty($address->adresseETR->complement)) {
        $adresse .= $address->adresseETR->complement."\n";
      }
      $adresse .= $address->adresseETR->codePostal;
      $adresse .= ' '.$address->adresseETR->ville;
      $adresse .= "\n";
      $adresse .= $address->adresseETR->etat."\n";rfc
      $adresse .= $address->adresseETR->pays."\n";
    } else {
      $adresse = FALSE;
    }

    return $adresse;
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
