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

/* Composite attributes that returns an empty value when all fields are empty */
class CompositeScriptAliasAttribute extends CompositeAttribute
{
  function writeValues($values)
  {
    if (($values[0] == "") && ($values[1] == "")) {
      return "";
    } else {
      return parent::writeValues($values);
    }
  }
}

class serviceApacheVhostEdit extends simplePlugin
{
  var $displayHeader  = FALSE;
  var $mainTab        = TRUE;
  var $objectclasses  = array("apacheConfig");
  var $cn             = "";

  /*!
   * \brief Plugin information
   */
  static function plInfo ()
  {
    return array(
      'plHeadline'    => _('Apache2'),
      'plShortName'   => _('Apache2'),
      'plDescription' => _('This Plugin is for Apache server'),
      'plObjectType'  => array('apacheVhost' => array(
        'name'        => _('Apache Vhost'),
        'filter'      => 'objectClass=apacheConfig',
        'aclCategory' => 'server',
        'icon'        => 'geticon.php?context=applications&icon=apache&size=16',
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'section1' => array (
        'name'  => _('Informations'),
        'class' => array('fullwidth'),
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
          new CompositeScriptAliasAttribute (
            _('Script Alias'),
            'apacheScriptAlias',
            array(
              new StringAttribute (_('Alias'),  _('Alias Directory'),   'aliasdir'),
              new StringAttribute (_('Script'), _('Script Directory'),  'scriptdir'),
            ),
            '/^(\S*) (\S*)$/',
            '%s %s',
            '',
            _('Script Alias')
          ),
          new SetAttribute (
            new CompositeAttribute (
              _('Server Alias'),
            'apacheServerAlias',
              array(
                new StringAttribute (_('URL Alias'),    _('URL Alias'),     'urlalias'),
                new StringAttribute (_('Directory Path'), _('Directory Path'),  'pathdir'),
              ),
              '/^(\S*) (\S*)$/',
              '%s %s',
              '',
              _('Server Alias')
            )
          )
        )
      )
    );
  }

  function __construct ($config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesInfo['section1']['attrs']['apacheScriptAlias']->setLinearRendering(TRUE);
  }

  function compute_dn ()
  {
    return "apacheServerName=".$this->apacheServerName.",".session::get('serviceApacheVhostEditDN');
  }

  function execute ()
  {
    $smarty = get_smarty();
    return parent::execute().$smarty->fetch(get_template_path('serviceApacheVhostEditFooter.tpl', TRUE, dirname(__FILE__)));
  }
}
?>
