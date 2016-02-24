<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2016  FusionDirectory

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

  class subContractingAccount extends simplePlugin
  {
    // We set displayHeader to FALSE, because we don't want a header allowing to activate/deactivate this plugin,
    // we want it activated on all objects
    var $displayHeader = TRUE;
  
    // Here we indicate which LDAP classes our plugin is using.
    var $objectclasses = array('fdSubcontractingPerson');

		function is_this_account($attrs)
		{
			$this->objectclasses  = array('fdSubcontractingPerson');
			$found = parent::is_this_account($attrs);
			return $found;
		}
  
    // The main function : information about attributes
    static function getAttributesInfo ()
    {
			return array(
				'section1' => array(
				'name'  => _('Sub Contractor information'),
				'attrs' => array(
					new StringAttribute (
						_('Final customer'),      // Label
						_('Final Customer'),      // Description
						'fdMissionFinalCustomer',       // LDAP name
						TRUE,                           // Mandatory
						''             // Default value
						),
					)
				),
			);
		}
  
    
    // We also need this function that return some information about the plugin
    static function plInfo ()
    {
      return array(
        'plShortName'       => _('Subcontracting'),
        'plDescription'     => _('Edit subcontracting infos'),
        'plSelfModify'      => TRUE,                  // Does this plugin have an owner that might be able to edit its entry
        'plObjectType'      => array('user'),
  
        // simplePlugin can generate the ACL list for us
        'plProvidedAcls'    => parent::generatePlProvidedAcls(self::getAttributesInfo())
      );
    }
  }
