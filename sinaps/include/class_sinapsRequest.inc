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

class sinapsRequestException extends Exception
{
}

class sinapsRequest
{
  protected $data;

  public function __construct($xml)
  {
    $this->data = new SimpleXMLElement($xml);
    if (!isset($this->data->domaine)) {
      throw new sinapsRequestException('XML content is missing "domaine" tag');
    }
    if (!isset($this->data->domaine['code'])) {
      throw new sinapsRequestException('"domaine" tag is missing "code" attribute');
    }
    if (!isset($this->data->domaine->donnees->operation)) {
      throw new sinapsRequestException('XML content is missing "domaine>donnees>operation" tag');
    }
    if (!isset($this->data->domaine->donnees->operation['codeOperation'])) {
      throw new sinapsRequestException('"operation" tag is missing "codeOperation" attribute');
    }
    if (!isset($this->data->domaine->identifiantTransaction)) {
      throw new sinapsRequestException('"identifiantTransaction" tag is missing from "domaine" tag');
    }
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

  public static function acquittementFonctionnel($responseCode = 200, $codeAcquittement = 0, $message = '', $identifiantObjApp = NULL, $synchrone = TRUE)
  {
    $xml = new SimpleXMLElement('<?xml version="1.0" encoding="UTF-8"?>
    <Acquittement xmlns:acq="http://referentiels.SINAPS.amue.fr/acquittementFonctionnel">
    </Acquittement>');
    $xml->addChild('ResponseCode', $responseCode);
    $xml->addChild('messageAcquittement', $message);
    $xml->addChild('codeAcquittement', $codeAcquittement);
    if ($identifiantObjApp) {
      $xml->addChild('identifiantObjApp', $identifiantObjApp);
    } else {
      $xml->addChild('identifiantObjApp');
    }

    if ($synchrone) {
      /* Hack because SINAPS API makes no sense and needs a prefix for Acquittement only for synchrone mode */
      return preg_replace(
        array('|<Acquittement |',   '|</Acquittement>$|'),
        array('<acq:Acquittement ', '</acq:Acquittement>'),
        $xml->asXML(),
        1
      );
    } else {
      return $xml->asXML();
    }
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

  public function getSupannEntiteValues($identifiantApplication, $identifiantApplicationSync, $uuidPrefix, $ldapUuidToCodeEntiteCallback)
  {
    $structure  = $this->getOperationStructure();
    $now        = new DateTime();

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
        'postalAddress'             => '',
        'supannCodeEntiteParent'    => '',
      ),
    );

    $uuid = '';

    if (isset($structure->referenceCroisee->identifiantsExternes)) {
      foreach ($structure->referenceCroisee->identifiantsExternes as $ref) {
        if ($ref->identifiantApplication == $identifiantApplication) {
          $uuid = $ref->identifiantExterne;
        } elseif (in_array($ref->identifiantApplication, $identifiantApplicationSync)) {
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
      } else {
        $values[$fdTab][$fdAttr] = '';
      }
    }

    if (isset($structure->methodesDeContact->methodeDeContact)) {
      foreach ($structure->methodesDeContact->methodeDeContact as $method) {
        if (!$this->checkValidite($method, $now)) {
          continue;
        }
        switch ((string)$method->codeTypeMethodeContact) {
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

    if (isset($structure->liensStructure->lienStructure)) {
      foreach ($structure->liensStructure->lienStructure as $lienStructure) {
        if (((string)$lienStructure->codeTypeLien == 'HIE') && isset($lienStructure->codeStructureMere)) {
          if (!$this->checkValidite($lienStructure, $now)) {
            continue;
          }
          $mereUUID = $this->getUUIDFromReferenceCroisee($identifiantApplication, $lienStructure->referenceCroisee);
          if (!empty($mereUUID)) {
            $values['entite']['supannCodeEntiteParent'] = $ldapUuidToCodeEntiteCallback($mereUUID)['supannCodeEntite'];
          }
        }
      }
    }

    return $values;
  }

  public function getUserValues($identifiantApplication, array $identifiantApplicationSync, $uuidPrefix, $ldapUuidToCodeEntiteCallback)
  {
    $personne   = $this->getOperationPersonne();
    $now        = new DateTime();
    $mapping = array(
      'civilite'        => array('supannAccount','supannCivilite'),
      'nomUsage'        => array('user','sn'),
      'descriptifLong'  => array('user','description'),
      'dateNaissance'   => array('personalInfo','dateOfBirth'),
      'sexe'            => array('personalInfo','gender'),
    );
    $values = array(
      'lock'          => FALSE,
      'user'          => array(
        'homePhone'                 => '',
        'postalAddress'             => '',
        'homePostalAddress'         => '',
        'employeeNumber'            => '',
      ),
      'supannAccount' => array(
        'supannRefId'                       => array(),
        'supannEntiteAffectation'           => array(),
        'supannRoleGenerique'               => array(),
        'supannTypeEntiteAffectation'       => array(),
        'supannRoleEntite'                  => array(),
        'supannActivite'                    => array(),
        'supannEmpCorps'                    => '',
        'supannEntiteAffectationPrincipale' => '',
      ),
      'personalInfo'  => array(
        'fdPrivateMail' => array(),
      ),
    );

    $uuid = '';

    if (isset($personne->referenceCroisee->identifiantsExternes)) {
      foreach ($personne->referenceCroisee->identifiantsExternes as $ref) {
        if ($ref->identifiantApplication == $identifiantApplication) {
          $uuid = $ref->identifiantExterne;
        } elseif (in_array($ref->identifiantApplication, $identifiantApplicationSync)) {
          $values['supannAccount']['supannRefId'][] = '{'.$ref->identifiantApplication.'}'.$ref->identifiantExterne;
        }
        if ($ref->identifiantApplication == 'SAP') {
          $values['user']['employeeNumber'] = $ref->identifiantExterne;
        }
      }
    }

    if (empty($uuid)) {
      $uuid = static::genUUID();
    }

    array_unshift($values['supannAccount']['supannRefId'], '{'.$uuidPrefix.'}'.$uuid);

    if (!isset($personne->rattachements->rattachement)) {
      /* This is a deletion */
      /* Store current date as end of contract */
      $values['personalInfo']['fdContractEndDate'] = $now->format('d.m.Y');
      /* Lock the account */
      $values['lock'] = TRUE;
      /* Remove all contact info */
      $values['user']['telephoneNumber']          = '';
      $values['user']['facsimileTelephoneNumber'] = '';
      $values['user']['mobile']                   = '';
      $values['mailAccount']                      = FALSE;
      return $values;
    }

    foreach ($mapping as $sinapsAttr => list($fdTab,$fdAttr)) {
      if (isset($personne->$sinapsAttr)) {
        $values[$fdTab][$fdAttr] = (string)$personne->$sinapsAttr;
      } else {
        $values[$fdTab][$fdAttr] = '';
      }
    }

    if (isset($personne->prenoms)) {
      $values['user']['givenName'] = preg_replace('/;.*$/', '', $personne->prenoms);
    }

    if (isset($personne->methodesDeContact->methodeContact)) {
      foreach ($personne->methodesDeContact->methodeContact as $method) {
        if (!$this->checkValidite($method, $now)) {
          continue;
        }
        switch ((string)$method->typeMethodeContact) {
          case 'TELPERSO':
            $values['user']['homePhone'] = (string)$method->valeur;
            break;
          case 'MAILPERSO':
            $values['personalInfo']['fdPrivateMail'][] = (string)$method->valeur;
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
    }

    if (isset($personne->rattachements->rattachement)) {
      foreach ($personne->rattachements->rattachement as $rattachement) {
        if (!$this->checkValidite($rattachement, $now)) {
          continue;
        }

        if (isset($rattachement->roles->role)) {
          foreach ($rattachement->roles->role as $role) {
            if (!$this->checkValidite($role, $now)) {
              continue;
            }
            if ($role->typeRole != 'PR') {
              continue;
            }
            if (isset($role->personneRessource->affectations->affectation)) {
              foreach ($role->personneRessource->affectations->affectation as $affectation) {
                if (!$this->checkValidite($affectation, $now)) {
                  continue;
                }
                $UUID = $this->getUUIDFromReferenceCroisee($identifiantApplication, $affectation->referenceCroisee);
                if (!empty($UUID)) {
                  $entite = $ldapUuidToCodeEntiteCallback($UUID);
                  $values['supannAccount']['supannEntiteAffectation'][]         = $entite['supannCodeEntite'];
                  $values['supannAccount']['supannEntiteAffectationPrincipale'] = $entite['supannCodeEntite'];
                }
              }
            }
            if (isset($role->personneRessource->elementsDeCarriere->elementCarriere)) {
              foreach ($role->personneRessource->elementsDeCarriere->elementCarriere as $elementCarriere) {
                if (!$this->checkValidite($elementCarriere, $now)) {
                  continue;
                }
                /* Warning: supannEmpCorps and employeeType are mono-value so we replace them */
                if (isset($elementCarriere->corps)) {
                  $values['supannAccount']['supannEmpCorps'] = (string)$elementCarriere->corps;
                }
                if (isset($elementCarriere->bapReferens)) {
                  $values['supannAccount']['supannActivite'][] = (string)$elementCarriere->bapReferens;
                }
              }
            }
          }
        }
        if (isset($rattachement->fonctionsOccupees->fonctionOccupee)) {
          foreach ($rattachement->fonctionsOccupees->fonctionOccupee as $fonctionOccupee) {
            if (!$this->checkValidite($fonctionOccupee, $now)) {
              continue;
            }
            $values['supannAccount']['supannRoleGenerique'][] = (string)$fonctionOccupee->fonctionRh;
            $UUID = $this->getUUIDFromReferenceCroisee($identifiantApplication, $fonctionOccupee->referenceCroisee);
            if (!empty($UUID)) {
              $entite = $ldapUuidToCodeEntiteCallback($UUID);
              $values['supannAccount']['supannTypeEntiteAffectation'][] = $entite['supannTypeEntite'];
              $values['supannAccount']['supannEntiteAffectation'][] = $entite['supannCodeEntite'];
              $values['supannAccount']['supannRoleEntite'][] =
                '[role='.$fonctionOccupee->fonctionRh.']'.
                '[type='.$entite['supannTypeEntite'].']'.
                '[code='.$entite['supannCodeEntite'].']';
            }
          }
        }
      }
    }

    foreach (array('supannRefId','supannEntiteAffectation','supannRoleGenerique','supannTypeEntiteAffectation','supannRoleEntite','supannActivite') as $arrayattribute) {
      $values['supannAccount'][$arrayattribute] = array_values(array_unique($values['supannAccount'][$arrayattribute]));
    }

    return $values;
  }

  protected function convertAddress(SimpleXMLElement $address)
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

  protected function checkValidite(SimpleXMLElement $object, DateTime $now)
  {
    if ($object->dateDebutValidite) {
      $dateDebutValidite = new DateTime($object->dateDebutValidite);
      if ($dateDebutValidite >= $now) {
        // Object is not valid yet, skip it
        return FALSE;
      }
    }
    if ($object->dateFinValidite) {
      $dateFinValidite = new DateTime($object->dateFinValidite);
      if ($dateFinValidite < $now) {
        // Object is not valid anymore, skip it
        return FALSE;
      }
    }
    return TRUE;
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

  public static function genUUID()
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
