<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2010-2012 Antoine Gallavardin
  Copyright (C) 2013-2016 FusionDirectory

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

class SupannPrefixedSelectAttribute extends CompositeAttribute
{
  protected $prefixedChoices;

  function __construct($label, $description, $ldapName, $required, $filename, $acl = "")
  {
    $attributes = array(
      new SelectAttribute('', '', $ldapName.'_prefix', $required),
      new SelectAttribute('', '', $ldapName.'_content', $required)
    );
    parent::__construct($description, $ldapName, $attributes, '/^{(.*)}(.*)$/', '{%s}%s', $acl, $label);
    $this->setLinearRendering(TRUE);
    $this->prefixedChoices = supann::get_prefixed_choices_for($filename);
    if (!$required) {
      $this->prefixedChoices[''] = array(array(''), array(_('None')));
    }
    $this->attributes[0]->setChoices(array_keys($this->prefixedChoices));
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->supannUpdateSelect();
    $this->setRequired($required);
  }

  protected function supannUpdateSelect()
  {
    $prefix = $this->attributes[0]->getValue();
    $this->attributes[1]->setChoices($this->prefixedChoices[$prefix][0], $this->prefixedChoices[$prefix][1]);
    $this->attributes[1]->setRequired($prefix != '');
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->supannUpdateSelect();
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->attributes[0]->setValue($values[0]);
    $this->supannUpdateSelect();
    $this->attributes[1]->setValue($values[1]);
  }

  function resetToDefault ()
  {
    $this->attributes[0]->resetToDefault();
    $this->supannUpdateSelect();
    $this->attributes[1]->resetToDefault();
  }

  function writeValues ($values)
  {
    if ($values[0] == '') {
      return '';
    } else {
      return parent::writeValues($values);
    }
  }

  function displayValue($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->setValue($values);
    $v1 = $this->attributes[0]->displayValue($values[0]);
    $v2 = $this->attributes[1]->displayValue($values[1]);
    return ($v1 == ''?$v2:$v1.': '.$v2);
  }
}

class SupannCursusAnneeAttribute extends SupannPrefixedSelectAttribute
{
  function __construct($label, $description, $ldapName, $required, $acl = "")
  {
    $attributes = array(
      new SelectAttribute('', '', $ldapName.'_prefix', $required),
      new SelectAttribute('', '', $ldapName.'_content', FALSE)
    );
    CompositeAttribute::__construct($description, $ldapName, $attributes, '/^{SUPANN}(.)(\\d+)$/', '{SUPANN}%s%d', $acl, $label);
    $this->setLinearRendering(TRUE);

    $this->attributes[0]->setChoices(
      array('L','M','D','X','B'),
      array(_('Licence'),_('Master'),_('Ph.D.'),_('Another class of degree'),_('post-graduate year'))
    );
    $yearLabels = array(
      _('None'),_('1st year'),_('2nd year'),_('3rd year'),
      _('4th year'),_('5th year'),_('6th year'),
      _('7th year'),_('8th year'),_('9th year'),
    );
    $this->prefixedChoices = array(
      'L' => array(range(0, 3),array_slice($yearLabels, 0, 3 + 1)),
      'M' => array(range(0, 2),array_slice($yearLabels, 0, 2 + 1)),
      'D' => array(range(0, 9),array_slice($yearLabels, 0, 10)),
      'X' => array(range(0, 9),array_slice($yearLabels, 0, 10)),
      'B' => array(range(0, 20),range(0, 20)),
    );
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->supannUpdateSelect();
    $this->setRequired($required);
  }
  protected function supannUpdateSelect()
  {
    $prefix = $this->attributes[0]->getValue();
    $this->attributes[1]->setChoices($this->prefixedChoices[$prefix][0], $this->prefixedChoices[$prefix][1]);
  }
}

class SupannCompositeAttribute extends CompositeAttribute
{
  function __construct($description, $ldapName, $attributes, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($description, $ldapName, $attributes, '', '', $acl, $label);
  }

  function readValues($value)
  {
    $values = array();
    $m = array();
    foreach ($this->attributes as &$attribute) {
      $shortname = preg_replace('/^[^_]+_/', '', $attribute->getLdapName());
      if (preg_match("/\\[$shortname=([^\\]]+)\\]/", $value, $m)) {
        $values[] = $m[1];
      } else {
        $values[] = "";
      }
    }
    unset($attribute);

    return $values;
  }

  function writeValues($values)
  {
    $value  = '';
    $i      = 0;
    foreach ($this->attributes as &$attribute) {
      if ($values[$i] != '') {
        $shortname = preg_replace('/^[^_]+_/', '', $attribute->getLdapName());
        $value .= "[$shortname=".$values[$i]."]";
      }
      $i++;
    }
    unset($attribute);
    return $value;
  }

  function supannGetValues(&$values)
  {
    foreach ($this->attributes as &$attribute) {
      $shortname = preg_replace('/^([^_]+)_.*$/', '\\1', $attribute->getLdapName());
      $value = $attribute->getValue();
      if (!isset($values[$shortname])) {
        $values[$shortname] = array();
      }
      if ($value == "") {
        continue;
      }
      $values[$shortname][$value] = $value;
    }
    unset($attribute);
  }
}

class SupannOrderedArrayAttribute extends OrderedArrayAttribute
{
  function supannPrepareSave()
  {
    $values = array();
    foreach ($this->value as $value) {
      $this->attribute->setValue($value);
      $this->attribute->supannGetValues($values);
    }
    foreach ($values as $ldapName => $array) {
      if ($this->plugin->attributesAccess[$ldapName]->isVisible()) {
        continue;
      }
      $this->plugin->attributesAccess[$ldapName]->setValue(array_values($array));
    }
  }
}

class supannAccount extends simplePlugin
{
  var $objectclasses = array('eduPerson', 'supannPerson');

  var $displayHeader = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('SupAnn'),
      'plDescription' => _('SupAnn informations management plugin'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann&size=16',
      'plSelfModify'  => TRUE,
      'plPriority'    => 14,
      'plObjectType'  => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    $year = intval(date('Y'));
    return array(
      'identity' => array(
        'name'  => _('Identity'),
        'attrs' => array(
          new SelectAttribute(
            _('Civilite'), _('supannCivilite - Civility for this person'),
            'supannCivilite', FALSE,
            array('', 'M.', 'Mme', 'Mlle')
          ),
          new StringAttribute(
            _('Alias login'), _('supannAliasLogin - An alias for the login of this user'),
            'supannAliasLogin', FALSE
          ),
          new StringAttribute(
            _('eduPersonPrincipalName'), _('eduPersonPrincipalName - A name that look like <id>@<domain> which is unique for this domain, and has not be assigned to anyone else recently'),
            'eduPersonPrincipalName', FALSE
          ),
          new StringAttribute(
            _('Nick name'), _('eduPersonNickname - Can contain a nick name for this user'),
            'eduPersonNickname', FALSE
          ),
          new SetAttribute(
            new StringAttribute(
              _('Ref ids'), _('supannRefId - IDs/links for this user on other systems'),
              'supannRefId', FALSE,
              '', '',
              '/^{[^}]+}.+$/' /* Validation regexp: it must have a prefix */
            )
          ),
        )
      ),
      'contact' => array(
        'name'  => _('Contact'),
        'attrs' => array(
          new SetAttribute(
            new StringAttribute(
              _('Other phone numbers'), _('supannAutreTelephone - Other phone numbers for this user'),
              'supannAutreTelephone', FALSE
            )
          ),
          new SetAttribute(
            new MailAttribute(
              _('Other mail addresses'), _('supannAutreMail - Other mail addresses for this user. Each must be unique'),
              'supannAutreMail', FALSE
            )
          ),
          new SetAttribute(
            new MailAttribute(
              _('Personal mail addresses'), _('supannMailPerso - Personal mail addresses for this user'),
              'supannMailPerso', FALSE
            )
          ),
          new BooleanAttribute(
            _('red List'), _('supannListeRouge - Should this person be on the red list'),
            'supannListeRouge', TRUE
          )
        )
      ),
      'affectation' => array(
        'name'  => _('Affectation'),
        'attrs' => array(
          new SelectAttribute(
            _('Entity primary assignment'), _('supannEntiteAffectationPrincipale - Main assignment of the person'),
            'supannEntiteAffectationPrincipale', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Entity assignment'), _('supannEntiteAffectation - represents assignments of the person in an institution, a component, service, etc.'),
              'supannEntiteAffectation', FALSE
            )
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Entity type assignment'), _('supannTypeEntiteAffectation'),
              'supannTypeEntiteAffectation', FALSE
            )
          ),
        )
      ),
      'affiliation' => array(
        'name'  => _('Affiliation'),
        'attrs' => array(
          new SelectAttribute(
            _('Primary affiliation'), _('eduPersonPrimaryAffiliation - Main status of the person'),
            'eduPersonPrimaryAffiliation', FALSE
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Affiliations'), _('eduPersonAffiliation - status of the person: student, BIATOSS, teacher, contract, retired, hosted staff (CNRS, INSERM, etc.), a former student, etc.'),
              'eduPersonAffiliation', FALSE
            )
          ),
          new SetAttribute(
            new SelectAttribute(
              _('Establishment'), _('supannEtablissement - institution or unit of administrative attachment of the person'),
              'supannEtablissement', FALSE
            )
          ),
        )
      ),
      'student' => array(
        'name'  => _('Student profile'),
        'attrs' => array(
          new StringAttribute(
            _('INE code'), _('supannCodeINE - INE code of this student'),
            'supannCodeINE', FALSE
          ),
          new StringAttribute(
            _('Student ID'), _('supannEtuId - Scolarity id'),
            'supannEtuId', FALSE
          ),
        )
      ),
      'student2' => array(
        'name'      => _('Student subscriptions'),
        'class'     => array('fullwidth'),
        'template'  => get_template_path('student_subscription.tpl', TRUE, dirname(__FILE__)),
        'attrs'     => array(
          /* These attributes are handled by the SupannOrderedArrayAttribute */
          new HiddenAttribute('supannEtuAnneeInscription'),
          new HiddenAttribute('supannEtuRegimeInscription'),
          new HiddenAttribute('supannEtuSecteurDisciplinaire'),
          new HiddenAttribute('supannEtuTypeDiplome'),
          new HiddenAttribute('supannEtuCursusAnnee'),
          new HiddenAttribute('supannEtuDiplome'),
          new HiddenAttribute('supannEtuEtape'),
          new HiddenAttribute('supannEtuElementPedagogique'),
          new SupannOrderedArrayAttribute(
            new SupannCompositeAttribute(
              _('supannEtuInscription - Subscriptions for this student'),
              'supannEtuInscription',
              array(
                new SelectAttribute(
                  _('Establishment'), _('supannEtablissement - Etablissement in which this subscription was done'),
                  'supannEtablissement_etab', TRUE
                ),
                new IntAttribute(
                  _('Year'), _('supannEtuAnneeInscription - The year this subscription will begin'),
                  'supannEtuAnneeInscription_anneeinsc', TRUE,
                  $year - 100, $year + 100, $year
                ),
                new SelectAttribute(
                  _('Subscription type'), _('supannEtuRegimeInscription - The type of this subscription'),
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
                  _('Curriculum year '), _('supannEtuCursusAnnee - type of curriculum (L, M, D or X, ...) and the year in the diploma.'),
                  'supannEtuCursusAnnee_cursusann', TRUE
                ),
                new SelectAttribute(
                  _('Entity assignment'), _('supannEntiteAffectation_affect'),
                  'supannEntiteAffectation_affect', FALSE
                ),
                new SupannPrefixedSelectAttribute(
                  _('Diploma'), _('supannEtuDiplome - Diploma prepared by the student'),
                  'supannEtuDiplome_diplome', FALSE, 'diplome'
                ),
                new SupannPrefixedSelectAttribute(
                  _('Step'), _('supannEtuEtape - Step can be considered a split (semester, year, etc.) in time of education leading to a diploma'),
                  'supannEtuEtape_etape', FALSE, 'etuetape'
                ),
                new SupannPrefixedSelectAttribute(
                  _('educational element'), _('supannEtuElementPedagogique - Generic description of the content of education with a high level of granularity'),
                  'supannEtuElementPedagogique_eltpedago', FALSE, 'etuelementpedagogique'
                ),
              )
            ),
            FALSE,  // no order
            array(),
            FALSE   // no edit button
          )
        )
      ),
      'personnal' => array(
        'name'  => _('Personal profile'),
        'attrs' => array(
          new StringAttribute(
            _('Personal ID'), _('supannEmpId - Employee identifier'),
            'supannEmpId', FALSE
          ),
          new SelectAttribute(
            _('Personal corps'), _('supannEmpCorps'),
            'supannEmpCorps', FALSE
          ),
          new SetAttribute(
            new SupannPrefixedSelectAttribute(
              _('Activity'), _('supannActivite - category of profession'),
              'supannActivite', FALSE, 'activite'
            )
          )
        )
      ),
      'personnal2' => array(
        'name'      => _('Roles'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          /* These attributes are handled by the SupannOrderedArrayAttribute */
          new HiddenAttribute('supannRoleGenerique'),
          new SupannOrderedArrayAttribute(
            new SupannCompositeAttribute(
              _('supannRoleEntite'),
              'supannRoleEntite',
              array(
                new SupannPrefixedSelectAttribute(
                  _('Generic role'), _('supannRoleGenerique - Generics Roles of the person in the facility'),
                  'supannRoleGenerique_role', TRUE, 'role'
                ),
                new SelectAttribute(
                  _('Entity assignment type'), _('supannTypeEntiteAffectation'),
                  'supannTypeEntiteAffectation_type', TRUE
                ),
                new SelectAttribute(
                  _('Entity assignment'), _('supannEntiteAffectation - represents assignments of the person in an institution, a component, service, etc.'),
                  'supannEntiteAffectation_code', FALSE
                ),
              ),
              '',
              '' // no label
            ),
            FALSE,  // no order
            array(),
            FALSE   // no edit button
          )
        )
      ),
    );
  }

  function __construct ($unused, $dn = NULL, $object = NULL)
  {
    global $config;
    parent::__construct($unused, $dn, $object);

    /* list of entity stored in LDAP tree */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=supannEntite)', array('supannCodeEntite', 'ou', 'o','supannEtablissement'));

    $code_entites   = array();
    $label_entites  = array();
    $code_etablissements  = array();
    $label_etablissements = array();
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['supannCodeEntite'][0])) {
        $code_entites[] = $attrs['supannCodeEntite'][0];
        if (isset($attrs['ou'][0])) {
          $label_entites[] = $attrs['ou'][0]." (".$attrs['supannCodeEntite'][0].")";
        } else {
          $label_entites[] = $attrs['o'][0]." (".$attrs['supannCodeEntite'][0].")";
        }
        if (isset($attrs['supannEtablissement'][0])) {
          $code_etablissements[]  = $attrs['supannEtablissement'][0];
          $label_etablissements[] = $attrs['o'][0]." (".$attrs['supannEtablissement'][0].")";
        }
      }
    }
    array_multisort($label_entites,         $code_entites);
    array_multisort($label_etablissements,  $code_etablissements);

    $this->attributesAccess['supannEntiteAffectation']->attribute->setChoices($code_entites, $label_entites);
    $this->attributesAccess['supannEtablissement']->attribute->setChoices($code_etablissements, $label_etablissements);
    $this->supannInit();
  }

  function supannInit()
  {
    list ($codes, $labels) = supann::get_choices_for('entite', '{SUPANN}');
    $this->attributesAccess['supannTypeEntiteAffectation']->attribute->setChoices($codes, $labels);

    list ($codes, $labels) = supann::get_choices_for('affiliation');
    $this->attributesAccess['eduPersonAffiliation']->attribute->setChoices($codes, $labels);
    $this->attributesAccess['supannEtuInscription']->setLinearRendering(FALSE);

    list ($codes, $labels) = supann::get_choices_for('eturegimeinscription_SISE', '{SISE}');
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[2]->setChoices($codes, $labels); // supannEtuRegimeInscription

    list ($codes, $labels) = supann::get_choices_for('discipline_SISE', '{SISE}');
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[3]->setChoices($codes, $labels); // supannEtuSecteurDisciplinaire

    list ($codes, $labels) = supann::get_choices_for('typediplome_SISE', '{SISE}');
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[4]->setChoices($codes, $labels); // supannEtuTypeDiplome

    list ($codes, $labels) = supann::get_choices_for('corps', '{NCORPS}');
    $this->attributesAccess['supannEmpCorps']->setChoices($codes, $labels);

    $this->updateFieldsChoices();
  }

  /* Update choices of fields which depends on other fields values */
  function updateFieldsChoices()
  {
    $code_ent   = $this->attributesAccess['supannEntiteAffectation']->getValue();
    $label_ent  = $this->attributesAccess['supannEntiteAffectation']->getDisplayValues();
    $this->attributesAccess['supannEntiteAffectationPrincipale']->setChoices(
      $code_ent, $label_ent
    );
    $code_etab   = $this->attributesAccess['supannEtablissement']->getValue();
    $label_etab  = $this->attributesAccess['supannEtablissement']->getDisplayValues();
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[0]->setChoices($code_etab, $label_etab); // supannEtablissement
    $this->attributesAccess['supannEtuInscription']->attribute->attributes[6]->setChoices($code_ent, $label_ent); // supannEntiteAffectation
    $this->attributesAccess['supannRoleEntite']->attribute->attributes[2]->setChoices($code_ent, $label_ent); // supannEntiteAffectation
    $code_tent   = $this->attributesAccess['supannTypeEntiteAffectation']->getValue();
    $label_tent  = $this->attributesAccess['supannTypeEntiteAffectation']->getDisplayValues();
    $this->attributesAccess['supannRoleEntite']->attribute->attributes[1]->setChoices($code_tent, $label_tent); // supannTypeEntiteAffectation

    $this->attributesAccess['eduPersonPrimaryAffiliation']->setChoices(
      $this->attributesAccess['eduPersonAffiliation']->getValue(),
      $this->attributesAccess['eduPersonAffiliation']->getDisplayValues()
    );
  }

  function save_object()
  {
    parent::save_object();
    $this->updateFieldsChoices();
  }

  protected function prepare_save ()
  {
    $this->attributesAccess['supannEtuInscription']->supannPrepareSave();
    $this->attributesAccess['supannRoleEntite']->supannPrepareSave();
    parent::prepare_save();
  }
}
?>
