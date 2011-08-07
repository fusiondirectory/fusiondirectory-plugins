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

class scalixGroup extends scalixAccount
{
  var $scalixGroup = TRUE;

  var $attributes           = array("scalixMailnode");
  var $fakeBooleanAttributes= array("scalixHideUserEntry");
  var $objectclasses        = array("scalixGroupClass");


  static function plInfo()
  {
    return (array(
          "plDescription"     => _("Scalix account"),
          "plSelfModify"      => TRUE,
          "plDepends"         => array("group"),
          "plPriority"        => 2,
          "plSection"         => array("administration"),
          "plCategory"        => array("groups"),
          "plOptions"         => array(),

          "plProvidedAcls"  => array(

            "scalixMailnode"            => _("Scalix Mail node"),
            "scalixMailboxClass"        => _("Mailbox class"),
            "scalixServerLanguage"      => _("Server language"),
            "scalixAdministrator"       => _("Administrator"),
            "scalixMailboxAdministrator"=> _("Mailbox administrator"),
            "scalixHideUserEntry"       => _("Hide user entry in Scalix"),
            "scalixLimitMailboxSize"    => _("Mailbox size limitations"),
            "scalixLimitOutboundMail"   => _("Limit outbound"),
            "scalixLimitInboundMail"    => _("Limit inbound"),
            "scalixLimitNotifyUser"     => _("Notify user"),
            "scalixEmailAddress"        => _("Scalix email addresses"))

            ));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler
?>
