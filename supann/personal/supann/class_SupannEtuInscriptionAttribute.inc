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

class SupannEtuInscriptionAttribute extends SupannCompositeAttribute
{
  protected $diplomas;

  function __construct ($description, $ldapName, $acl = '', $label = 'Composite attribute')
  {
    $year = intval(date('Y'));
    $attributes = [
      new SelectAttribute(
        _('Establishment'), _('supannEtablissement - Etablissement in which this registration was done'),
        'supannEtablissement_etab', TRUE
      ),
      new IntAttribute(
        _('Year'), _('supannEtuAnneeInscription - The year this registration will begin'),
        'supannEtuAnneeInscription_anneeinsc', TRUE,
        $year - 100, $year + 100, $year
      ),
      new SelectAttribute(
        _('Registration type'), _('supannEtuRegimeInscription - The type of this registration'),
        'supannEtuRegimeInscription_regimeinsc', TRUE
      ),
      new SelectAttribute(
        _('Disciplinary Sector'), _('supannEtuSecteurDisciplinaire - Disciplinary sector education diploma'),
        'supannEtuSecteurDisciplinaire_sectdisc', TRUE
      ),
      new SelectAttribute(
        _('Diploma type'), _('supannEtuTypeDiplome - Type of diploma'),
        'supannEtuTypeDiplome_typedip', TRUE
      ),
      new SupannCursusAnneeAttribute(
        _('Curriculum year '), _('supannEtuCursusAnnee - Type of curriculum (L, M, D or X, ...) and the year in the diploma.'),
        'supannEtuCursusAnnee_cursusann', TRUE
      ),
      new SelectAttribute(
        _('Entity assignment'), _('supannEntiteAffectation - To wich entities does this user belong to'),
        'supannEntiteAffectation_affect', FALSE
      ),
      new SupannPrefixedSelectAttribute(
        _('Diploma'), _('supannEtuDiplome - Diploma prepared by the student'),
        'supannEtuDiplome_diplome', FALSE, ''
      ),
      new SupannPrefixedSelectAttribute(
        _('Step'), _('supannEtuEtape - Step can be considered a split (semester, year, etc.) in time of education leading to a diploma'),
        'supannEtuEtape_etape', FALSE, 'etuetape'
      ),
      new SupannPrefixedSelectAttribute(
        _('educational element'), _('supannEtuElementPedagogique - Generic description of the content of education with a high level of granularity'),
        'supannEtuElementPedagogique_eltpedago', FALSE, 'etuelementpedagogique'
      ),
    ];
    parent::__construct($description, $ldapName, $attributes, $acl, $label);

    /* supannEtuRegimeInscription */
    list ($codes, $labels) = supann::get_choices_for('eturegimeinscription_SISE', '{SISE}');
    $key = array_search('{SISE}$', $codes);
    if ($key !== FALSE) {
      /* Move "empty" value as first one */
      array_unshift($codes,   ...array_splice($codes,   $key, 1));
      array_unshift($labels,  ...array_splice($labels,  $key, 1));
    }
    $this->attributes[2]->setChoices($codes, $labels);

    /* supannEtuSecteurDisciplinaire */
    list ($codes, $labels) = supann::get_choices_for('discipline_SISE', '{SISE}');
    $key = array_search('{SISE}-', $codes);
    if ($key !== FALSE) {
      /* Move "empty" value as first one */
      array_unshift($codes,   ...array_splice($codes,   $key, 1));
      array_unshift($labels,  ...array_splice($labels,  $key, 1));
    }
    $this->attributes[3]->setChoices($codes, $labels);
    $this->attributes[3]->setSubmitForm(TRUE);

    /* supannEtuTypeDiplome */
    list ($codes, $labels) = supann::get_choices_for('typediplome_SISE', '{SISE}');
    $this->attributes[4]->setChoices($codes, $labels);
    $this->attributes[4]->setSubmitForm(TRUE);

    $prefixedChoices = supann::get_prefixed_choices_for(
      'diplome',
      function ($line)
      {
        return explode(';', $line);
      }
    );

    $this->diplomas = [];
    foreach ($prefixedChoices as $prefix => $choices) {
      list($choicesId, $choicesLines) = $choices;
      foreach ($choicesId as $i => $id) {
        list($type,$sectdisc,$label1,$label2) = $choicesLines[$i];
        if (!empty($label2)) {
          $label1 .= ' - '.$label2;
        }
        if ($type[0] != '{') {
          $type = '{'.$prefix.'}'.$type;
        }
        if ($sectdisc[0] != '{') {
          $sectdisc = '{'.$prefix.'}'.$sectdisc;
        }
        $this->diplomas[$type][$sectdisc][$prefix][0][] = $id;
        $this->diplomas[$type][$sectdisc][$prefix][1][] = $label1;
      }
    }

    $this->supannUpdateSelect();
  }

  protected function supannUpdateSelect ()
  {
    $sectdisc = $this->attributes[3]->getValue();
    $type     = $this->attributes[4]->getValue();
    $this->attributes[7]->setPrefixedChoices($this->diplomas[$type][$sectdisc] ?? []);
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    $this->supannUpdateSelect();
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    reset($values);
    foreach ($this->attributes as &$attribute) {
      if ($attribute->getLdapName() == 'supannEtuDiplome_diplome') {
        $this->supannUpdateSelect();
      }
      $attribute->setValue(current($values));
      next($values);
    }
    unset($attribute);
    reset($values);
  }

  function resetToDefault ()
  {
    foreach ($this->attributes as &$attribute) {
      if ($attribute->getLdapName() == 'supannEtuDiplome_diplome') {
        $this->supannUpdateSelect();
      }
      $attribute->resetToDefault();
    }
    unset($attribute);
  }

  /* Remove SupAnn weirdness */
  function readValues (string $value): array
  {
    $values = parent::readValues($value);

    if ($values[2] == '{INCONNU}') {
      $values[2] = '';
    }
    if ($values[3] == '{INCONNU}') {
      $values[3] = '';
    }

    return $values;
  }

  /* Add {INCONNU} values for special cases regimeinsc and sectdisc */
  function writeValues (array $values)
  {
    if ($values[2] == '') {
      $values[2] = '{INCONNU}';
    }
    if ($values[3] == '') {
      $values[3] = '{INCONNU}';
    }
    return parent::writeValues($values);
  }
}
