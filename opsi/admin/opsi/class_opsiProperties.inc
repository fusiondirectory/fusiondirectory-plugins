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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*! \brief  Displays opsi product properties.
 */
class opsiProperties extends plugin
{
  private $product;
  private $cfg;
  private $hostId;
  public $config;

  /*! \brief  Initializes the class.
      @param  Object  The gosa configuration object
      @param  String  The product name.
      @param  Array   The product config (e.g. aray('attr' => 'value'))
      @param  String  The host id if necessary
   */
  public function __construct($config,$product,$cfg = array() ,$hostId = "")
  {
    $this->config  = $config;
    $this->product = $product;
    $this->cfg     = $cfg;
    $this->hostId  = $hostId;
  }


  /*! \brief  Display html dialog which allows to 
                configure the properties
      @return String  HTML content
   */
  public function execute()
  {
    $ui =get_userinfo();
    $acl =  $ui->get_permissions($this->config->current['BASE'],"opsi/opsiProperties","");
    $smarty = get_smarty();
    $smarty->assign("ACL",$acl);
    $smarty->assign("cfg",$this->cfg);
    $smarty->assign("cfg_count",count($this->cfg));
    return($smarty->fetch(get_template_path("properties.tpl",TRUE,dirname(__FILE__))));
  }


  /*! \brief  Save the posted property updates 
   */
  public function save_object()
  {
    $ui =get_userinfo();
    $acl =  $ui->get_permissions($this->config->current['BASE'],"opsi/opsiProperties","");
    if(preg_match("/w/",$acl)){
      foreach($this->cfg as $name => $value){
        if(isset($_POST['value_'.$name])){
          $this->cfg[$name]['CURRENT'] = validate(get_post('value_'.$name));
        }
      }
    }
  }


  /*! \brief  Sets the current config array.
   */
  public function set_cfg($cfg) 
  {
    $this->cfg = $cfg;
  }


  /*! \brief  Returns the current product config
      @return Array.
   */
  public function get_cfg() 
  {
    return($this->cfg);
  }

  
  /*! \brief  Returns the ID of the product.
      @param  String  Product ID
   */
  public function get_product()
  {
    return($this->product);
  }

  
  /*! \brief  Sets the current product ID, 
              The name is only used to remember the currently edited product.
   */
  public function set_product($name)
  {
    $this->product = $name;
  }


  /*! \brief  Retuns the hostId.
   */
  public function get_hostId()
  {
    return($this->hostId);
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Products"),
          "plDescription" => _("Product properties"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 10,
          "plSection"     => array("administration"),
          "plCategory"    => array("opsi"),
          "plProvidedAcls"=> array(
            )
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
