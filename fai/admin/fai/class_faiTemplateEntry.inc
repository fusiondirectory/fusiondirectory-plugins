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

class AccessRightsAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $acl = "")
  {
    $attributes = array(
      'u4' => new BooleanAttribute('', '', 'ur', FALSE, TRUE),
      'u2' => new BooleanAttribute('', '', 'uw', FALSE, TRUE),
      'u1' => new BooleanAttribute('', '', 'ue', FALSE),
      's4' => new BooleanAttribute('', '', 'us', FALSE),
      'g4' => new BooleanAttribute('', '', 'gr', FALSE, TRUE),
      'g2' => new BooleanAttribute('', '', 'gw', FALSE),
      'g1' => new BooleanAttribute('', '', 'ge', FALSE),
      's2' => new BooleanAttribute('', '', 'gs', FALSE),
      'o4' => new BooleanAttribute('', '', 'or', FALSE),
      'o2' => new BooleanAttribute('', '', 'ow', FALSE),
      'o1' => new BooleanAttribute('', '', 'oe', FALSE),
      's1' => new BooleanAttribute('', '', 'os', FALSE),
    );
    parent::__construct ($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
    $this->setLinearRendering(TRUE);
  }

  function renderFormInput()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->setDisabled($this->disabled);
    }
    unset($attribute);

    $display = '<table style="border:1px solid #B0B0B0">
         <colgroup width="55" span="3">
          </colgroup>
    <tr>
      <th>{t}Class{/t}</th>
      <th>{t}Read{/t}</th>
      <th>{t}Write{/t}</th>
      <th>{t}Execute{/t}</th>
      <th>&nbsp;</th>
      <th>{t}Special{/t}</th>
      <th>&nbsp;</th>
    </tr>
    <tr><td>{t}User{/t}</td>
      <td align="center">'.$this->attributes['u4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['u2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['u1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s4']->renderFormInput().'</td>
      <td>({t}SUID{/t})</td>
      </tr>

    <tr><td>{t}Group{/t}</td>
      <td align="center">'.$this->attributes['g4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['g2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['g1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s2']->renderFormInput().'</td>
      <td>({t}SGID{/t})</td>
      </tr>

    <tr><td>{t}Others{/t}</td>
      <td align="center">'.$this->attributes['o4']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['o2']->renderFormInput().'</td>
      <td align="center">'.$this->attributes['o1']->renderFormInput().'</td>
      <td>&nbsp;</td>
      <td align="center">'.$this->attributes['s1']->renderFormInput().'</td>
      <td>({t}sticky{/t})</td>
    </tr></table>';

    return $display;
  }

  function readValues($value)
  {
    $tmode  = "$value ";
    $m      = array();
    foreach (array('s', 'u', 'g', 'o') as $type) {
      $current  = substr($tmode, 0, 1);
      $tmode    = preg_replace('/^./', '', $tmode);
      $nr       = 1;
      while ($nr < 5) {
        $m[$type.$nr] = ($current & $nr?'TRUE':'FALSE');
        $nr += $nr;
      }
    }
    $values = array();
    foreach (array_keys($this->attributes) as $name) {
      $values[] = $m[$name];
    }
    return $values;
  }

  function writeValues($values)
  {
    $tmode  = '';
    foreach (array('s', 'u', 'g', 'o') as $type) {
      $nr   = 1;
      $dest = 0;
      while ($nr < 5) {
        if ($values[$type.$nr] === 'TRUE') {
          $dest += $nr;
        }
        $nr += $nr;
      }
      $tmode = $tmode.$dest;
    }
    return $tmode;
  }
}

class faiTemplateEntry extends simplePlugin
{
  var $mainTab        = TRUE;
  var $objectclasses  = array('top', 'FAIclass', 'FAItemplateEntry');

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new PathAttribute (
            _('File path'), _('Complete absolute template file path and name'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Description'), _('Short description of the file'),
            'description', FALSE
          ),
          new HiddenAttribute (
            'FAItemplatePath', TRUE
          ),
        )
      ),
      'template' => array(
        'name'  => _('Template attributes'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new FileTextAreaAttribute (
            _('Template file content'), _('Content of the template file'),
            'FAItemplateFile', TRUE
          ),
          new CompositeAttribute (
            _('Owner and group of the file'), 'FAIowner',
            array(
              new StringAttribute (
                _('Owner'), _('File owner'),
                'owner', TRUE,
                'root'
              ),
              new StringAttribute (
                _('Group'), _('File group'),
                'group', TRUE,
                'root'
              ),
            ),
            '/^([^.]+)\.([^.]+)$/',
            '%s.%s'
          ),
          new AccessRightsAttribute (
            _('Access'), _('Access rights'),
            'FAImode'
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }

  function save_object()
  {
    if (isset($_FILES[$this->attributesAccess['FAItemplateFile']->getHtmlId()]['name'])) {
      $name = $_FILES[$this->attributesAccess['FAItemplateFile']->getHtmlId()]['name'];
    }
    parent::save_object();
    if (preg_match('/^\/?$/', $this->cn) && isset($name)) {
      $this->cn = '/'.$name;
    }

    $this->FAItemplatePath = $this->cn;
  }
}
?>
