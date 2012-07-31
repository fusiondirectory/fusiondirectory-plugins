<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class serviceApacheVhostEdit extends simplePlugin
{

  var $displayHeader = FALSE;
  var $mainTab       = TRUE;
  var $objectclasses = array("apacheConfig");
  var $cn             = "";

  /*!
   * \brief Plugin information
   */
  static function plInfo ()
  {
    return array(
    "plHeadline"        => _("Apache2"),
    "plShortName"       => _("Apache2"),
    "plDescription"     => _("This Plugin is for Apache serveur"),
    "plSelfModify"      => FALSE,                  // Does this plugin have an owner that might be able to edit its entry
    "plSection"         => array("administration"),
    "plCategory"        => array("server"),
    "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'section1' => array (
        'name'  => _('Generic'),
        'attrs' => array(
          new StringAttribute (
            _('Virtual host name'),
            _('The Virtual host name'),
            'apacheServerName',
            TRUE
          ),
          new StringAttribute (
            _('Document root'),
            _('Document root for the apache web server'),
            'apacheDocumentRoot',
            TRUE
          ),
          new StringAttribute (
            _('Admin mail address'),
            _('Apache admin mail address'),
            'apacheServerAdmin',
            TRUE
          ),
          new SetAttribute (
             new CompositeAttribute (
              _('Script Alias'),
              'apacheScriptAlias',
              array(
                new StringAttribute (_('Alias Directory'),    _('Alias Directory'),     'aliasdir'),
                new StringAttribute (_('Script Directory'), _('Script Directory'),  'scriptdir'),
              ),
              '%s %s',
              '%s %s',
              '',
              _('Script Alias')
            )
          ),
          new SetAttribute (
            new CompositeAttribute (
              _('Server Alias'),
            'apacheServerAlias',
              array(
                new StringAttribute (_('URL Alias'),    _('URL Alias'),     'urlalias'),
                new StringAttribute (_('Directory Path'), _('Directory Path'),  'pathdir'),
              ),
              '%s %s',
              '%s %s',
              '',
              _('Server Alias')
            )
          )
        )
      )
    );
  }

/*
  function save ()
  {
    echo "JE ME SAUVEGARDE";
    parent::save();
  }
*/

  function __construct ($config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  function execute ()
  {
    $smarty = get_smarty();
    return parent::execute().$smarty->fetch(get_template_path('serviceApacheVhostEditFooter.tpl', TRUE, dirname(__FILE__)));
  }
}
?>
