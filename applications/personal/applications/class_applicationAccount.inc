<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2018  FusionDirectory
  Copyright (C) 2018  Antoine Gallavardin

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


/*!
  \brief   application plugin

  This class provides the functionality to read and write all informations 
  for creating / editing / deleting remote seafiel account.

 */
 
/* class for selection application */


class ApplicationSelectAttribute extends CompositeAttribute
{
  protected $applicationChoices;

  function __construct($label, $description, $ldapName, $required, $filename, $acl = "")
  {
    global $config;
    $attributes = array(
      new SelectAttribute('', '', $ldapName.'_app', $required),
      new SelectAttribute('', '', $ldapName.'_role', $required)
    );


    parent::__construct($description, $ldapName, $attributes, '/^{(.*)}(.*)$/', '{%s}%s', $acl, $label);
    $this->setLinearRendering(TRUE);
    /* list of entity stored in LDAP tree */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdWebApplication)', array('cn', 'fdApplicationAvailablesProfiles'));

    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['cn'][0])) {
	 $app = $attrs['cn'][0];
	 $allRolesCode=[];
	 $allRolesDesc=[];
	foreach($attrs['fdApplicationAvailablesProfiles'] as $val){
		$splitRoles=explode('|',$val);
		$allRolesCode[]=$splitRoles[0];
		$allRolesDesc[]=$splitRoles[1];
	        	
	}
      }
      $this->applicationChoices[$app]=array($allRolesCode,$allRolesDesc);
    }
    
    if (!$required) {
      $this->applicationChoices[''] = array(array(''), array(_('None')));
    }
    $this->attributes[0]->setChoices(array_keys($this->applicationChoices));
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->applicationUpdateSelect();
    $this->setRequired($required);
  }

  protected function applicationUpdateSelect()
  {
    $prefix = $this->attributes[0]->getValue();
    if (!isset($this->applicationChoices[$prefix])) {
      $this->applicationChoices[$prefix] = array(array(), array());
    }
    $this->attributes[1]->setChoices($this->applicationChoices[$prefix][0], $this->applicationChoices[$prefix][1]);
    $this->attributes[1]->setRequired($prefix != '');
  }

  function applyPostValue()
  {
    parent::applyPostValue();
    $this->applicationUpdateSelect();
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $this->attributes[0]->setValue($values[0]);
    $this->applicationUpdateSelect();
    $this->attributes[1]->setValue($values[1]);
  }

  function resetToDefault ()
  {
    $this->attributes[0]->resetToDefault();
    $this->applicationUpdateSelect();
    $this->attributes[1]->resetToDefault();
  }

  function writeValues (array $values)
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
    $choices2 = $this->attributes[1]->getDisplayChoices();
    if (isset($choices2[$values[1]])) {
      $v2 = $choices2[$values[1]];
      } else {
      $v2 = $values[1];
    }
    return ($v1 == '' ? $v2 : $v1.': '.$v2);
  }
}



 


class applicationAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdApplicationAccount');
  
  private $mainSectionAttrs = array();


  static function plInfo()
  {
    return array(
      'plShortName'     => _('Profile'),
      'plDescription'   => _('Profile per Application'),
      'plIcon'          => 'geticon.php?context=types&icon=application&size=48',
      'plSmallIcon'     => 'geticon.php?context=types&icon=application&size=16',
      'plDepends'       => array(),
      'plSelfModify'    => FALSE,
      'plPriority'      => 4,
      'plObjectType'    => array('user'),
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => array(
      )
    );
  }

  /*!
  *  \brief The main function : information about attributes
   */


  static function getAttributesInfo ()
  {


    return array (
      'main' => array (
        'name'  => _('Applications account'),
	'attrs' => array (
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Profiles per each applications'),
              'fdApplicationProfiles',
              array(
                new ApplicationSelectAttribute(
                  _('Profiles'), _('Applications Profiles - Profiles available for each applications'),
                  'fdApplicationProfiles', TRUE, 'webApplication'
                  ),
                )
              ),
              // no order
              FALSE,
              array(),
              // edit button
              TRUE
            )
          )
        ),
      
    );
  }


  function execute()
  {
    $smarty = get_smarty();

    global $config;

    return parent::execute();
  }

}

?>
