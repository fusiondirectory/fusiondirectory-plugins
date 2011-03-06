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

class opsiWare extends plugin
{
  private $opsi;
  private $hostId;
  private $init_failed = FALSE;
  private $info;
  protected $type = 0;

  public function __construct($config,$hostId)
  {
    $this->config = $config;
    $this->opsi   = new opsi($config);
    $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);
    $this->init();
  }

  private function init()
  {
    if($this->type == 0){
      $this->info = $this->opsi->get_client_hardware($this->hostId);
      $err = $this->opsi->is_error(); 
    }else{
      $this->info = $this->opsi->get_client_software($this->hostId);
      $err = $this->opsi->is_error(); 
    }
    $this->init_failed = $err;
  }


  public function execute()
  {
    if(isset($_POST['reinit']) && $this->init_failed){
      $this->init();
    }
    $smarty = get_smarty();
    $smarty->assign("init_failed",$this->init_failed);
    if($this->init_failed){
      $smarty->assign("type", $this->type);
      $smarty->assign("message",$this->opsi->get_error());
      return($smarty->fetch(get_template_path("opsiware.tpl",TRUE,dirname(__FILE__))));
    }  
    $smarty->assign("type", $this->type);
    $smarty->assign("info", $this->info);
    return($smarty->fetch(get_template_path("opsiware.tpl",TRUE,dirname(__FILE__))));
  }
}


class opsiSoftware extends opsiWare
{
  protected $type = 0;
}

class opsiHardware extends opsiWare
{
  protected $type = 1;
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
