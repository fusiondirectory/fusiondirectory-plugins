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
 
 


class applicationAccount extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array('fdApplicationAccount');
  var $myclient	= '';
  
  private $mainSectionAttrs = array();
  
  static function plInfo()
  {
    return array(
      'plShortName'     => _('Applications'),
      'plDescription'   => _('applications settings'),
      'plIcon'          => 'geticon.php?context=applications&icon=seafile&size=48',
      'plSmallIcon'     => 'geticon.php?context=applications&icon=seafile&size=16',
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
      
          new CompositeAttribute (
            _('Informations for ftp login'),
              'ftpLoginInfo',
            array(
              new StringAttribute (_('Login'),    _('Login for FTP'),     'ftpLogin'),
              new StringAttribute (_('Password'), _('Password for FTP'),  'ftpPassword'),
              new StringAttribute (_('Host'),     _('Host for FTP'),      'ftpHost'),
              new IntAttribute    (_('Port'),     _('Port for FTP'),      'ftpPort', FALSE, 0, FALSE, 21),
            ),
          'ftp://%[^@:]:%[^@:]@%[^@:]:%d', // sscanf format
          'ftp://%s:%s@%s:%d'              // sprintf format
        ),
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
