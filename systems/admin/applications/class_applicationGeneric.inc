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

class ScriptAttribute extends FileAttribute
{
  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if ($this->isVisible()) {
      foreach ($_POST as $name => $value) {
        if (preg_match('/^download'.$this->getHtmlId().'/',$name)) {
          session::set('binary', $this->value);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->plugin->cn.".fdApplication");
          header('location: getbin.php');
          exit();
        }
      }
      if (isset($_POST['upload'.$this->getHtmlId()])) {
        parent::loadPostValue();
      } else {
        $id = $this->getHtmlId().'_text';
        if (isset($_POST[$id])) {
          $this->setPostValue($_POST[$id]);
        }
      }
    }
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible()) {
      parent::applyPostValue();
      if (is_object($this->plugin)) {
        $this->setValue($this->inputValue($this->getValue()));
      }
    }
  }

  function inputValue ($value)
  {
    if (is_object($this->plugin)) {
      $value = $this->plugin->generateTemplate().preg_replace('/.*### END HEADER ###/s', '', $value);
    }
    return $value;
  }

  function renderFormInput ()
  {
    $this->setValue($this->inputValue($this->getValue()));
    $id = $this->getHtmlId();
    $display  = '<textarea name="'.$id.'_text" id="'.$id.'_text"'.
                ($this->disabled? 'disabled="disabled"':'').'>'.
                '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}</textarea>';
    $display  .= '<input type="file" name="'.$id.'" id="'.$id.'" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    $display  .= '<input type="submit" name="upload'.$id.'" id="upload'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="'._('Upload').'"/>';
    $display  .= '<input type="image" name="download'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' src="images/save.png" title="'._('Download').'" class="center"/>';
    return $this->renderAcl($display);
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array($id.'_text',$id,'upload'.$id,'download'.$id);
  }

  function computeLdapValue()
  {
    $value = parent::computeLdapValue();
    if (preg_replace('/.*### END HEADER ###/s', '', $value) == '') {
      return '';
    } else {
      return $value;
    }
  }
}

class IconAttribute extends FileAttribute
{
  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if (isset($_POST['upload'.$this->getHtmlId()])) {
      parent::loadPostValue();
    }
  }

  function setValue ($value)
  {
    if ($value == "") {
      $this->value = "";
      return;
    }
    if (class_exists('Imagick')) {
      $im = new Imagick();
      $im->readImageBlob($value);

      $size = $im->getImageGeometry();

      if (($size['width'] != 0 && $size['height'] != 0) && ($size['width'] != 48 && $size['height'] != 48)) {
        $im->resizeImage(48, 48, Imagick::FILTER_GAUSSIAN, 1, FALSE);
      }
      $im->setImageFormat('png');

      /* Save attribute */
      $this->value = $im->getImageBlob();
    } else {
      msg_dialog::display(_("Error"),
                _("Cannot save user picture, FusionDirectory requires the package 'php5-imagick' to be installed!"),
                ERROR_DIALOG);
    }
  }

  function renderFormInput ()
  {
    $this->setValue($this->inputValue($this->getValue()));
    $id = $this->getHtmlId();
    srand((double)microtime()*1000000); // Just to be sure the image is not cached
    $display  = '<img name="'.$id.'_img" id="'.$id.'_img"'.
                ($this->disabled? 'disabled="disabled"':'').
                ' src="getbin.php?rand='.rand(0, 10000).'"'.
                ' style="vertical-align:middle;margin-right:10px;border:1px solid black;" />';
    $display  .= '<input type="file" name="'.$id.'" id="'.$id.'" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    $display  .= '<input type="submit" name="upload'.$id.'" id="upload'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="'._('Upload').'"/>';
    session::set('binary', $this->getValue());
    session::set('binarytype', 'image/png');
    return $this->renderAcl($display);
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array($id.'_img',$id,'upload'.$id);
  }
}

class application extends simplePlugin
{
  var $objectclasses = array("top", "gosaApplication");

  var $mainTab = TRUE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Generic'),
      'plDescription' => _('Application generic'),
      'plSelfModify'  => FALSE,
      'plCategory'    => array('application' => array('description'  => _('Application'),
                                                      'objectClass'  => 'gosaApplication')),
      'plObjectType'  => array('application' => array('name'  => _('Application'),
                                                      'filter'  => 'objectClass=gosaApplication')),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new HostNameAttribute (
            _('Application name'), _('The name of this application'),
            'cn', TRUE
          ),
          new StringAttribute (
            _('Display name'), _('The displayed name for this application'),
            'gosaApplicationName', FALSE
          ),
          new StringAttribute (
            _('Execute'), _(''),
            'gosaApplicationExecute', TRUE
          ),
          new StringAttribute (
            _('Description'), _('A short description of this application'),
            'description', FALSE
          ),
          new BaseSelectorAttribute(get_ou('applicationRDN')),
        )
      ),
      'icon' => array(
        'name'  => _('Icon'),
        'attrs' => array(
          new IconAttribute (
            '', _('The icon for this application'),
            'gosaApplicationIcon'
          ),
        )
      ),
      'options' => array(
        'name'  => _('Options'),
        'attrs' => array(
          new BooleanAttribute(
            _('Only executable for member'), _(''),
            'flag_group', FALSE, FALSE, '',
            'G', ''
          ),
          new BooleanAttribute(
            _('Replace user configuration on startup'), _(''),
            'flag_overwrite', FALSE, FALSE, '',
            'O', ''
          ),
          new BooleanAttribute(
            _('Place icon on members desktop'), _(''),
            'flag_desktop', FALSE, FALSE, '',
            'D', ''
          ),
          new BooleanAttribute(
            _('Place entry in members startmenu'), _(''),
            'flag_menu', FALSE, FALSE, '',
            'M', ''
          ),
          new BooleanAttribute(
            _('Place entry in members launch bar'), _(''),
            'flag_launchbar', FALSE, FALSE, '',
            'L', ''
          ),
          new FlagsAttribute(
            'gosaApplicationFlags',
            array('flag_group','flag_overwrite','flag_desktop','flag_menu','flag_launchbar')
          ),
        )
      ),
      'scripton' => array(
        'name'  => _('Script'),
        'attrs' => array(
          new ScriptAttribute(
            '', _(''),
            'gotoLogonScript'
          ),
        )
      ),
      'scriptoff' => array(
        'name'  => _('Log off script'),
        'attrs' => array(
          new ScriptAttribute(
            '', _(''),
            'gotoLogoffScript'
          ),
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['cn']->setUnique(TRUE);
    $this->attributesAccess['base']->setDisabled($this->is_release());

    $filename = "./plugins/systems/images/default_icon.png";

    if (file_exists($filename)) {
      $fd = fopen ($filename, "rb");
      $this->attributesAccess['gosaApplicationIcon']->setDefaultValue(fread ($fd, filesize ($filename)));
      fclose ($fd);
      if (empty($this->gosaApplicationIcon)) {
        $this->attributesAccess['gosaApplicationIcon']->resetToDefault();
      }
    }
  }

  function execute()
  {
    $this->attributesAccess['base']->setDisabled($this->is_release());
    return parent::execute();
  }

  function is_release()
  {
    if (isset($this->parent->parent)) {
      return $this->parent->parent->IsReleaseManagementActivated();
    } else {
      /* Check if we should enable the release selection */
      return class_available("faiManagement");
    }
  }

  function generateTemplate()
  {
    if (!isset($this->parent->by_object['applicationParameters'])) {
      return "";
    }
    $str = "# This code is part of FusionDirectory http://www.fusiondirectory.org/\n#\n";

    $values = array();
    $names  = array();
    if ($this->parent->by_object['applicationParameters']->is_account) {
      $names  = $this->parent->by_object['applicationParameters']->option_name;
      $values = $this->parent->by_object['applicationParameters']->option_value;
    }

    if (count($names)) {
      $str .= "# This plugin handles these environment variables:\n";
    } else {
      $str .= "# This plugin handles no environment variables.\n";
    }

    foreach($names as $index => $name){

      // Fix length
      for($i = strlen($name) ; $i < 30 ; $i++){
        $name= $name." ";
      }
      if((isset($values[$index]))&&(!empty($values[$index]))){
        $str.= "# ".$name."\t(e.g. '".$values[$index]."')\n";
      }else{
        $str.= "# ".$name."\t("._("no example").")\n";
      }
    }
    $str .= "#\n".
      "# Don't remove the following tag, it is used for header update.\n".
      "### END HEADER ###";

    return($str);
  }

  function remove_from_parent()
  {
    if (!$this->initially_was_account) {
      return;
    }

    if ($this->acl_is_removeable()) {
      /* Parse release out of object dn */
      $release = preg_replace("/".preg_quote(get_ou('applicationRDN').$this->base, '/')."$/i","",$this->dn);
      $release = preg_replace("/^cn=[^,]+,/","",$release);

      /* Get a list of all groups
       */
      $groups = array();
      $ldap= $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(objectClass=posixGroup)",array("dn"));
      while($attrs = $ldap->fetch()){
        $groups[$attrs['dn']] = array();
      }

      /* Check if there are groups, useing this application
       */
      $found = array();
      foreach($groups as $group => $data){
        $ldap->cd($release.$group);
        $ldap->search("(objectClass=gotoMenuEntry)",array("dn"));
        while($attrs = $ldap->fetch()){
          $info = preg_replace("/".preg_quote($release.$group, '/')."$/","",$attrs['dn']);
          if(preg_match("/^cn=".$this->cn."/",$info) && !preg_match("/ou=[^,]+,/",$info)){
            $found[] = $attrs['dn'];
          }
        }
      }

      /* Create an error message an skip remove, if
          this application is still in use.
       */
      if(count($found)){
        msg_dialog::display(_("Error"), sprintf(_("Cannot remove application - it is still in use by these objects: %s"), "<br>".msgPool::buildList($found)), ERROR_DIALOG);
        return(FALSE);
      }

      parent::remove_from_parent();

      /* Delete references to object groups */
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      while ($ldap->fetch()){
        $og= new ogroup($this->config, $ldap->getDN());
        unset($og->member[$this->dn]);
        $og->save ();
        if (!$ldap->success()){
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()), ERROR_DIALOG);
        }
      }
    }
  }

}

?>
