<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class faiPackageConfiguration extends simplePlugin
{
  var $ignore_account = TRUE;

  protected $packageName;
  protected $debconfAttributesTypes;
  protected $generatedDebconfAttributes;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('FAI package configuration'),
      'plDescription' => _('Configure debconf options of a package'),
      'plObjectType'  => array('faiPackageConfiguration' => array(
        'name'        => _('FAI package configuration'),
        'aclCategory' => 'fai',
        'icon'        => 'geticon.php?context=applications&icon=fai&size=16',
      )),

      'plProvidedAcls'  => array()
    );
  }

  /*! \brief  Configure a FAI package entry.
      @param  String  The DN of the current package.
      @param  String  The package name (e.g. apache)
      @param  String  The release name (e.g. edge)
      @param  Array   The current package configuration.
   */
  function __construct($unused, $dn = NULL, $packageName, $release, $values)
  {
    $this->packageName  = $packageName;

    /* Read pkg debconf info from supportDaemon */
    $q = new supportDaemon();
    $ret = $q->FAI_get_packages($release, array('package','template'), array($this->packageName), 0, 1, TRUE);
    if ($q->is_error()) {
      msg_dialog::display(_('Infrastructure service'), msgPool::siError($q->get_error()), ERROR_DIALOG);
    }

    $tpl = '';
    if (isset($ret[0]) && isset($ret[0]['TEMPLATE'])) {
      $tpl = base64_decode($ret[0]['TEMPLATE']);
    }

    $attributesInfo = $this->loadAttributesFromTemplate($tpl);

    parent::__construct($unused, $dn, NULL, $attributesInfo);

    foreach ($values as $var) {
      $this->attributesAccess[$var['Name']]->setValue($var['Value']);
    }
  }

  protected function loadAttributesFromTemplate($tpl)
  {
    $attributesInfo = array(
      'main' => array(
        'name'  => sprintf(_('Debconf information for package "%s"'), $this->packageName),
        'attrs' => array()
      ),
    );

    $lines                  = explode("\n", $tpl);
    $infos                  = array();
    $langcode               = session::global_get('lang').'.UTF-8';
    $in_description         = FALSE;
    $got_local_description  = FALSE;

    $this->debconfAttributesTypes     = array();
    $this->generatedDebconfAttributes = array();

    foreach ($lines as $line) {
      /* Reset description flag */
      if ($in_description && !preg_match('/^ /', $line)) {
        $in_description = FALSE;
      }

      /* Template header */
      if (preg_match('/^Template: /', $line)) {
        if (!empty($infos)) {
          $this->insertDebconfAttribute($attributesInfo, $infos);
        }
        $infos = array();
        $infos['Name']     = trim(preg_replace('/^Template: /', '', $line));
        $infos['Default']  = '';

        $got_local_description = FALSE;
        continue;
      }

      /* Get type */
      if (preg_match('/^Type: /', $line)) {
        $infos['Type'] = trim(preg_replace('/^Type: /', '', $line));
        continue;
      }

      /* Get default */
      if (preg_match('/^Default: /', $line)) {
        $infos['Default'] = trim(preg_replace('/^Default: /', '', $line));
        continue;
      }

      /* Get description */
      if (!$got_local_description && preg_match('/^Description: /i', $line)) {
        $infos['Topic']       = trim(preg_replace('/^Description: /i', '', $line));
        $infos['Description'] = '';
        $in_description       = TRUE;
        continue;
      }

      /* Fill description */
      if (!$got_local_description && $in_description) {
        $infos['Description'] .= $line;
        continue;
      }

      /* Get local description */
      if (preg_match('/^Description-'.$langcode.': /i', $line)) {
        $infos['Topic'] = trim(preg_replace('/^Description-'.$langcode.': /i', '', $line));
        $in_description         = TRUE;
        $got_local_description  = TRUE;
        $infos['Description']   = '';
        continue;
      }

      /* Fill local description */
      if ($got_local_description && $in_description) {
        $infos['Description'] .= $line;
        continue;
      }

      /* Get native choices */
      if (preg_match('/^Choices: /', $line)) {
        $infos['Choices'] = trim(preg_replace('/^Choices: /', '', $line));
      }

      /* Get local choices */
      if (preg_match('/^Choices-'.$langcode.': /', $line)) {
        $infos['Choices-local'] = trim(preg_replace('/^Choices-'.$langcode.': /', '', $line));
      }
    }

    /* Add the last one */
    if (!empty($infos)) {
      $this->insertDebconfAttribute($attributesInfo, $infos);
    }

    if (!empty($this->generatedDebconfAttributes)) {
      msg_dialog::display(
        _('Warning'),
        sprintf(_('The debconf questions "%s" are dynamically generated during package installation and requires choosing between specific options which cannot be presented here. The entered text needs to be one of the valid choices in order to take effect.'), implode('","', $this->generatedDebconfAttributes)),
        WARNING_DIALOG
      );
    }

    return $attributesInfo;
  }

  protected function insertDebconfAttribute(&$attributesInfo, $infos)
  {
    switch ($infos['Type']) {
      case 'boolean':
        $attribute =
          new BooleanAttribute(
            $infos['Topic'], $infos['Description'],
            $infos['Name'], FALSE,
            $infos['Default'], 'FAIpackage',
            'true', 'false'
          );
        break;
      case 'password':
        $attribute =
          new PasswordAttribute(
            $infos['Topic'], $infos['Description'],
            $infos['Name'], FALSE,
            $infos['Default'], 'FAIpackage'
          );
        break;
      case 'multiselect':
      case 'select':
        if (!preg_match('/\$\{/', $infos['Choices'])) {
          $choices = explode(', ', $infos['Choices']);
          if (isset($infos['Choices-local'])) {
            $outputs = explode(', ', $infos['Choices-local']);
          } else {
            $outputs = $choices;
          }
          $attribute =
            new SelectAttribute(
              $infos['Topic'], $infos['Description'],
              $infos['Name'], FALSE,
              $choices, $infos['Default'], $outputs,
              'FAIpackage'
            );
          if ($infos['Type'] == 'multiselect') {
            $attribute = new CommaListAttribute(
              $infos['Name'],
              new SetAttribute(
                $attribute
              )
            );
          }
          break;
        } else {
          /* Issue a warning about generated choices and show a StringAttribute */
          $this->generatedDebconfAttributes[$infos['Name']] = $infos['Topic'];
        }
      case 'string':
        $attribute =
          new StringAttribute(
            $infos['Topic'], $infos['Description'],
            $infos['Name'], FALSE,
            $infos['Default'], 'FAIpackage'
          );
        break;
      case 'note':
        /* Ignore notes, they do not makes sense, since we don't get any
           chance to test entered values... */
      case 'text':
        /* Ignore text messages, they are normally used for status hints. */
      case 'title':
      case 'error':
        return;
      default:
        trigger_error('Unknown type '.$infos['Type'].'!');
        return;
    }
    $attribute->setInLdap(FALSE);
    $attributesInfo['main']['attrs'][]            = $attribute;
    $this->debconfAttributesTypes[$infos['Name']] = $infos['Type'];
  }

  function execute()
  {
    $smarty = get_smarty();
    $acl    = $this->getacl('FAIpartitions');
    $smarty->assign('FAIpackageACL', $acl);
    return parent::execute().
      '<p class="plugbottom">'.
      '  <input type="submit" name="SaveObjectConfig" value="'.msgPool::applyButton().'"/>'.
      '  <input type="submit" name="CancelObjectConfig" value="'.msgPool::cancelButton().'"/>'.
      '</p>';
  }

  function save()
  {
    $tmp = array();
    foreach ($this->attributesAccess as $name => $attribute) {
      $tmp[$this->packageName][$name] = array(
        'Name'  => $name,
        'Value' => $attribute->computeLdapValue(),
        'Type'  => $this->debconfAttributesTypes[$name],
      );
    }

    return $tmp;
  }
}
?>
