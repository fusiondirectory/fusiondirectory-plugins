<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class TerminalReleaseAttribute extends SelectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $choices = array())
  {
    parent::__construct($label, $description, $ldapName, $required, $choices);
    $this->setSubmitForm(TRUE);
  }

  function setValue ($value)
  {
    if (!in_array($value, $this->choices)) {
      return;
    }
    parent::setValue($value);
    if (isset($this->plugin)) {
      $this->plugin->attributesInfo['main']['attrs']['gotoBootKernel']->setRelease($value);
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $this->setValue('squeeze');
  }
}

class terminalStartup extends simplePlugin
{
  /* attribute list for save action */
  var $objectclasses = array("GOhard");

  var $inheritance = array("gosaGroupOfNames" => "member");

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'startup' => array (
        'name' => _('Startup parameters'),
        'attrs' => array (
          new StringAttribute (
            _('Boot kernel'),  _('Linux kernel to be installed and booted'),
            'gotoBootKernel', FALSE
          ),
          new StringAttribute (
            _('Boot kernel parameters'), _('Linux kernel parameters to be ran'),
            'gotoKernelParameters', FALSE
          )
        )
      ),
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Startup'),
      'plDescription'   => _('Terminal startup'),
      'plSelfModify'    => FALSE,
      'plPriority'      => 3,
      'plObjectType'    => array('terminal', 'ogroup-dynamic'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}

?>
