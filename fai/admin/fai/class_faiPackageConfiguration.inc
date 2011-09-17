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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class faiPackageConfiguration extends plugin
{
  /* attribute list for save action */
  var $ignore_account = TRUE;
  var $attributes     = array();
  var $objectclasses  = array();

  var $Debconf_handle = NULL;
  var $pkg_config ;
  var $obj;
  var $release;


  /*! \brief  Configure a FAI package entry.
      @param  Object  $config FusionDirectory configuration object;
      @param  String  The DN of the current package.
      @param  String  The package name (e.g. apache)
      @param  String  The release name (e.g. edge)
      @param  Array   The current package configuration.
   */
  function faiPackageConfiguration (&$config, $dn= NULL,$obj,$release,$pkg_config)
  {
    plugin::plugin ($config, $dn);
    $this->obj = $obj;
    $this->release = $release;
    $this->pkg_config = $pkg_config;

    /* Read pkg debconf info from supportDaemon
     */
    $q = new supportDaemon();
    $ret = $q->FAI_get_packages($this->release,array("package","template"),array("^".$this->obj."$"));
    if($q->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($q->get_error()),ERROR_DIALOG);
    }

    /* If we do not get a useable answer, use an empty template 
     */
    $tpl = "";
    if(isset($ret[0]) && isset($ret[0]['TEMPLATE'])){
      $tpl = base64_decode($ret[0]['TEMPLATE']);
    }

    /* Create package configuration mask
     */
    $this->Debconf_handle = new debconf(preg_replace('/_.*$/', '', get_browser_language()));
    $this->Debconf_handle->load_from_string($tpl);
    if($this->Debconf_handle->has_template()){
      foreach($this->pkg_config as $var){
        $this->Debconf_handle->SetDefault($var['Name'],$var['Value']);
      }
    }
  }


  /*! \brief  Returns a html interface, that allow to modify 
               the package attributes.
      @return String  HTML content/ui for this plugin
   */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $smarty->assign("headline", sprintf(_("Debconf information for package '%s'"), $this->obj));
    $this->Debconf_handle->PostCheck();
    $smarty->assign("Config",$this->Debconf_handle->get_dialog());
    return($smarty->fetch(get_template_path('faiPackageConfiguration.tpl', TRUE)));
  }


  /* Save data to object */
  function save_object()
  {
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
    return ($message);
  }


  /*! \brief  Returns an array with all configured attributes 
      @return Array   All configured attributes.
   */
  function save()
  {
    $this->Debconf_handle->PostCheck();
    $pkgs = $this->Debconf_handle->template;
    $tmp = array();
    foreach($pkgs as $pkg){
      $tmp[$this->obj][$pkg['Name']]['Value']= $pkg['Default'];
      $tmp[$this->obj][$pkg['Name']]['Name'] = $pkg['Name'];
      $tmp[$this->obj][$pkg['Name']]['Type'] = $pkg['Type'];
    }
    return($tmp);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
