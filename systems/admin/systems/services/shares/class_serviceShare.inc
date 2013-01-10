<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class serviceShare extends simpleService
{
  var $objectclasses = array('goShareServer');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'template'  => get_template_path('service_share.tpl', TRUE, dirname(__FILE__)),
        'name'      => _('Shares'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Shares this server hosts'),
              'goExportEntry',
              array(
                new StringAttribute(
                  _('Name'), _('Name of this share'),
                  'shareName', TRUE
                ),
                new StringAttribute(
                  _('Description'), _('Description of this share'),
                  'shareDesc', FALSE
                ),
                new StringAttribute(
                  _('Path / Volume'), _('Path or volume concerned by this share'),
                  'sharePath', TRUE
                ),
                new SelectAttribute(
                  _('Type'), _('Type of share'),
                  'shareType', TRUE,
                  array("CIFS","NBD","NCP","NFS","netatalk","samba")
                ),
                new SelectAttribute(
                  _('Codepage'), _('Codepage for this share'),
                  'shareCodepage', TRUE, array()
                ),
                new StringAttribute(
                  _('Option'), _('Special option(s) for this share'), 'shareOption'
                ),
              ),
              '', '' // no formats
            ),
            FALSE,  // no order
            array(),
            TRUE    // edit button
          )
        )
      )
    );
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Share service'),
      'plDescription' => _('Share service').' ('._('Services').')',

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $parent = NULL)
  {
    parent::__construct($config, $dn, $parent);
    $this->attributesAccess['goExportEntry']->setLinearRendering(FALSE);

    $charsets = $this->getCharsets();
    $this->attributesAccess['goExportEntry']->attribute->attributes[4]->setChoices(
      array_keys($charsets), array_values($charsets)
    );
  }

  function getCharsets()
  {
    $charsets = array();

    if (!file_exists(SYSTEMS_DIR."/encodings")) {
      msg_dialog::display(_("Error"), msgPool::fileDoesNotExist(SYSTEMS_DIR."/encodings"), ERROR_DIALOG);
    } else {
      if (!is_readable(SYSTEMS_DIR."/encodings")) {
        msg_dialog::display(_("Error"), msgPool::cannotReadFile(SYSTEMS_DIR."/encodings"), ERROR_DIALOG);
      } else {
        $fp = fopen(SYSTEMS_DIR."/encodings", "r");
        $i  = 100;
        while (!feof($fp) && $i) {
          $i--;
          $str = trim(fgets($fp, 256));

          /* Skip comments */
          if (!preg_match("/^#/", $str)) {
            $arr = explode("=", $str);
            if (count($arr) == 2) {
              $charsets[$arr[0]] = $arr[1];
            }
          }
        }
      }
    }
    return $charsets;
  }
}
?>
