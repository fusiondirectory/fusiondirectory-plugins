{if $is_account ne 'true'}
 <img alt="" class="center" src="images/small-error.png" align="middle">
 {t}The environment extension is currently disabled.{/t}
{else}

<input type="hidden" name="iamposted" value="1"/>

<table summary="{t}Environment managment settings{/t}" width="100%">
 <tr>
  <td style="width:50%;border-right:1px solid #B0B0B0">
  </td>
  <td style="vertical-align:top">
   <h2>
    <img class="center" alt="" src="plugins/systems/images/logon_script.png" align="middle" />&nbsp;
    <label for="gotoLogonScript">{t}Logon scripts{/t}</label>
   </h2>
   <table summary="{t}Logon script management{/t}" style="width:100%">
    <tr>
     <td>
{render acl=$gotoLogonScriptACL}
      <select style="width:100%;" name="gotoLogonScript" multiple size=5 id="gotoLogonScript">

    {if $multiple_support}
      {foreach from=$gotoLogonScripts item=item key=key}
        {if $item.UsedByAllUsers}
        <option value="{$key}">{$item.LogonPriority}&nbsp;{$item.LogonName}&nbsp;[{$item.LogonDescription}] ({t}Used by all users{/t})</option>
        {else}
        <option style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;' value="{$key}">{$item.LogonPriority}&nbsp;{$item.LogonName}&nbsp;[{$item.LogonDescription}] ({t}Used by some users{/t})</option>
        {/if}
      {/foreach}
    {else}
         {html_options values=$gotoLogonScriptKeys output=$gotoLogonScripts}
         <option disabled>&nbsp;</option>
    {/if}
      </select>
{/render}

      <br>
{render acl=$gotoLogonScriptACL}
      <input type="submit" name="gotoLogonScriptNew" value="{msgPool type=addButton}">
{/render}
{render acl=$gotoLogonScriptACL}
      <input type="submit" name="gotoLogonScriptEdit"  value="{t}Edit{/t}" {if $gotoLogonScriptKeysCnt ==0} disabled {/if}>
{/render}
{render acl=$gotoLogonScriptACL}
      <input type="submit" name="gotoLogonScriptDel"  value="{msgPool type=delButton}" {if $gotoLogonScriptKeysCnt ==0} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>

   <h3><label for="gotoLogoffScript">{t}Log off scripts{/t}</label></h3>
   <table summary="{t}Log off script management{/t}" style="width:100%">
    <tr>
     <td>
      {render acl=$gotoLogoffScriptACL}
      <select style="width:100%;" name="gotoLogoffScript" multiple size=5 id="gotoLogoffScript">

       {if $multiple_support}
       {foreach from=$gotoLogoffScripts item=item key=key}
       {if $item.UsedByAllUsers}
       <option value="{$key}">{$item.LogoffPriority}&nbsp;{$item.LogoffName}&nbsp;[{$item.LogoffDescription}] ({t}Used by all users{/t})</option>
       {else}
       <option style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;' value="{$key}">{$item.LogoffPriority}&nbsp;{$item.LogoffName}&nbsp;[{$item.LogoffDescription}] ({t}Used by some users{/t})</option>
       {/if}
       {/foreach}
       {else}
       {html_options values=$gotoLogoffScriptKeys output=$gotoLogoffScripts}
       <option disabled>&nbsp;</option>
       {/if}
      </select>
      {/render}

      <br>
      {render acl=$gotoLogoffScriptACL}
      <button type='submit' name='gotoLogoffScriptNew'>{msgPool type=addButton}</button>

      {/render}
      {render acl=$gotoLogoffScriptACL}
      <button type='submit' name='gotoLogoffScriptEdit' {if $gotoLogoffScriptKeysCnt ==0} disabled {/if}
      >{t}Edit{/t}</button>

      {/render}
      {render acl=$gotoLogoffScriptACL}
      <button type='submit' name='gotoLogoffScriptDel' {if $gotoLogoffScriptKeysCnt ==0} disabled {/if}
      >{msgPool type=delButton}</button>

      {/render}
     </td>
    </tr>
   </table>



  </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<table summary="{t}Environment managment settings{/t}" width="100%">
 <tr>
  <td style="border-right:1px solid #B0B0B0; width:50%; vertical-align:top">
   <h2>
    <img alt="" src="plugins/systems/images/hotplug.png" align="middle" class="center" />&nbsp;
    <label for="gotoHotplugDevice_post">{t}Hotplug devices{/t}</label>
   </h2>
   <table style="width:100%" summary="{t}Hotplug device settings{/t}">
    <tr>
     <td>
{render acl=$gotoHotplugDeviceACL}
      <select name="gotoHotplugDevice_post[]" size=5  style="width:100%;" id="gotoHotplugDevice_post" multiple>
    {if $multiple_support}
      {foreach from=$gotoHotplugDevices item=item key=key}
        {if $item.UsedByAllUsers}
        <option value="{$key}">{$item.name}&nbsp;[{$item.description}] ({t}Used by all users{/t})</option>
        {else}
        <option style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;' value="{$key}">{$item.name}&nbsp;[{$item.description}] ({t}Used by some users{/t})</option>
        {/if}
      {/foreach}
    {else}
       {html_options values=$gotoHotplugDeviceKeys output=$gotoHotplugDevices}
       <option disabled>&nbsp;</option>
    {/if}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td>
{render acl=$gotoHotplugDeviceACL}
      <input type="submit" name="gotoHotplugDeviceUse" value="{msgPool type=addButton}">
{/render}
{render acl=$gotoHotplugDeviceACL}
      <input type="submit" name="gotoHotplugDeviceDel" value="{msgPool type=delButton}"
      {if !$gotoHotplugDevices} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td>


{if $multiple_support}

   <h2>
  <input type="checkbox" name="use_gotoPrinter" value="1" {if $use_gotoPrinter} checked {/if}
    class="center" onClick="$('div_gotoPrinter').toggle();">
    <img alt="" src="plugins/systems/images/select_printer.png" align="middle" class="center" />&nbsp;
    <label for="gotoPrinter">{t}Printer{/t}</label>
   </h2>

   <div id="div_gotoPrinter" {if !$use_gotoPrinter} style="display: none;" {/if}>
  <b>{t}Using this option will overwrite the complete printer settings for all currently edited objects!{/t}</b>
   <table style="width:100%" summary="{t}Printer settings{/t}">
    <tr>
     <td>
{render acl=$gotoPrinterACL}
      <select style="width:100%;" name="gotoPrinterSel[]" multiple size=5 id="gotoPrinter">
       {html_options options=$gotoPrinter}
       <option disabled>&nbsp;</option>
      </select>
{/render}
      <br>
{render acl=$gotoPrinterACL}
      <input type="submit"  name="gotoPrinterAdd"     value="{msgPool type=addButton}">
{/render}
{render acl=$gotoPrinterACL}
      <input type="submit" name="gotoPrinterDel"     value="{msgPool type=delButton}" {if !$gotoPrinter} disabled {/if}>
{/render}
{render acl=$gotoPrinterACL}
      <input type="submit" name="gotoPrinterEdit"    value="{t}Toggle admin{/t}" {if !$gotoPrinter} disabled {/if}>
{/render}
{render acl=$gosaDefaultPrinterACL}
      <input type="submit" name="gotoPrinterDefault"    value="{t}Toggle default{/t}" {if !$gotoPrinter||$is_group} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>
  </div>

{else}

   <h2>
    <img alt="" src="plugins/systems/images/select_printer.png" align="middle" class="center" />&nbsp;
    <label for="gotoPrinter">{t}Printer{/t}</label>
   </h2>
   <table style="width:100%" summary="{t}Printer settings{/t}">
    <tr>
     <td>
{render acl=$gotoPrinterACL}
      <select style="width:100%;" name="gotoPrinterSel[]" multiple size=5 id="gotoPrinter">
       {html_options options=$gotoPrinter}
       <option disabled>&nbsp;</option>
      </select>
{/render}
      <br>
{render acl=$gotoPrinterACL}
      <input type="submit"  name="gotoPrinterAdd"     value="{msgPool type=addButton}">
{/render}
{render acl=$gotoPrinterACL}
      <input type="submit" name="gotoPrinterDel"     value="{msgPool type=delButton}" {if !$gotoPrinter} disabled {/if}>
{/render}
{render acl=$gotoPrinterACL}
      <input type="submit" name="gotoPrinterEdit"    value="{t}Toggle admin{/t}" {if !$gotoPrinter} disabled {/if}>
{/render}
{render acl=$gosaDefaultPrinterACL}
      <input type="submit" name="gotoPrinterDefault"    value="{t}Toggle default{/t}" {if !$gotoPrinter||$is_group} disabled {/if}>
{/render}
     </td>
    </tr>
   </table>

{/if}

  </td>
 </tr>
</table>
{/if}

