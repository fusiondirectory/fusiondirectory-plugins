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
  \brief   seafile plugin

  This class provides the functionality to read and write all informations 
  for creating / editing / deleting remote seafiel account.

 */
 
/* class for selection application */


class ApplicationSelectAttribute extends CompositeAttribute
{
  protected $applicationChoices;

  //global $config; 

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
    $ldap->search('(objectClass=fdWebApplication)', array('cn', 'fdApplicationAvailablesRoles'));

    //parcours des ebregistremens
    while ($attrs = $ldap->fetch()) {
      if (isset($attrs['cn'][0])) {
	 $app = $attrs['cn'][0];
	 $allRolesCode=[];
	 $allRolesDesc=[];
	foreach($attrs['fdApplicationAvailablesRoles'] as $val){
		$splitRoles=explode('|',$val);
		$allRolesCode[]=$splitRoles[0];
		$allRolesDesc[]=$splitRoles[1];
	        	
	}
      }
      $allchoices[$app]=array($allRolesCode,$allRolesDesc);
    }
    print_r($allchoices);
    print_r("<br>");

   // $allApplications = objects::ls('webApplication',array('cn'=>1,'fdApplicationTitle' => 1, 'fdApplicationAvailablesRoles' => '*' ),NULL,'(objectClass=fdWebApplication)');
   //$allApplications = objects::ls('webApplication');
    //var_dump($allApplications);
    //var_dump(objects);

//    $allchoices=array();
//    $allRolesCode=array();
//    $allRolesDesc=array();
/*    foreach($allApplications as $key => $value){
	    $allRolesCode=[];
	    $allRolesDesc=[];
	    foreach( $value['fdApplicationAvailablesRoles'] as $k => $v){
		    $splitRoles=explode("|",$v);
		    $allRolesCode[]=$splitRoles[0];
		    $allRolesDesc[]=$splitRoles[1];
	    }
      	$allchoices[]=array($value =>array($allRolesCode,$allRolesDesc));
    }

    var_dump($allchoices); */



    /*  $this->applicationChoices = FIXME: list des application */
    /* a requeter depsui la bracnhe */
    $this->applicationChoices = $allchoices;

    //$this->applicationChoices = array("color" => array(array("blue", "red", "green"),array("bleu","rouge","vert")),
//	    "size" => array(array("small", "medium", "large"),array("petit","moyen","grand")));

    print_r($this->applicationChoices);
    
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
  var $myclient	= '';
  
  private $mainSectionAttrs = array();


/*
  function getApplicationsRoles()
  {
    $allApplications = objects::ls('webApplication',array('cn'=>1,'fdApplicationTitle' => 1, 'fdApplicationAvailablesRoles' => '*' ),NULL,'(objectClass=fdWebApplication)');
    //var_dump($allApplications);

    $allchoices=array();
    $allRolesCode=array();
    $allRolesDesc=array();
    foreach($allApplications as $key => $value){
            $allRolesCode=[];
	    $allRolesDesc=[];
	    foreach( $value['fdApplicationAvailablesRoles'] as $k => $v){
		    //print("val : ".$v);
                    $splitRoles=explode("|",$v);
                    $allRolesCode[]=$splitRoles[0];
		    $allRolesDesc[]=$splitRoles[1];
		    //print("\nval :".$splitRoles[0]. " => ".$splitRoles[1]);
	    }
//	print("<br>");
//	    var_dump($allRolesCode);
//	    print("<br>");
//	var_dump($allRolesDesc);
        $allchoices[]=array($value['cn'] =>array($allRolesCode,$allRolesDesc));
    }

    return $allchoices;
    //var_dump($allchoices);
    /* Get List of server with LDAP service */
/*    return objects::ls('server', NULL, NULL, '(objectClass=goLdapServer)');*/
/*
}
*/
  


  
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Applications'),
      'plDescription'   => _('applications settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=applications&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=applications&size=16',
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
              _('Roles per each applications'),
              'fdApplicationRoles',
              array(
                new ApplicationSelectAttribute(
                  _('Roles'), _('Applications Roles - roles available for each applications'),
                  'fdApplicationRoles', TRUE, 'webApplication'
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

    #
    # Fetch Seafile Server
    #
    global $config;

    return parent::execute();
  }
  



  function prepare_save(){
	parent::prepare_save();
	global $config;
	
}




}

?>
