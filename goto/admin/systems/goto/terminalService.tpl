<table style="width:100%;" summary="">
 <tr>
  <td style="width:33%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="plugins/goto/images/keyboard.png"> {t}Keyboard{/t}</h2>
   <table summary="">
    <tr>
     <td><LABEL for="gotoXKbModel">{t}Model{/t}</LABEL></td>
     <td>
{render acl=$gotoXKbModelACL}
      <select id="gotoXKbModel" name="gotoXKbModel" title="{t}Choose keyboard model{/t}" >
       {html_options options=$XKbModels selected=$gotoXKbModel_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoXKbLayout">{t}Layout{/t}</LABEL></td>
     <td>
{render acl=$gotoXKbLayoutACL}
      <select id="gotoXKbLayout" name="gotoXKbLayout" title="{t}Choose keyboard layout{/t}" >
       {html_options options=$XKbLayouts selected=$gotoXKbLayout_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoXKbVariant">{t}Variant{/t}</LABEL></td>
     <td>
{render acl=$gotoXKbVariantACL}
      <select id="gotoXKbVariant" name="gotoXKbVariant" title="{t}Choose keyboard variant{/t}" >
       {html_options options=$XKbVariants selected=$gotoXKbVariant_select}
      </select>
{/render}
     </td>
    </tr>
   </table>

  </td>

  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  
  <td style="vertical-align:top;width:32%">
   <h2><img class="center" alt="" align="middle" src="plugins/goto/images/mouse.png"> {t}Mouse{/t}</h2>
   <table summary="">
    <tr>
     <td><LABEL for="gotoXMouseType">{t}Type{/t}</LABEL></td>
     <td>
{render acl=$gotoXMouseTypeACL}
      <select name="gotoXMouseType" id="gotoXMouseType" title="{t}Choose mouse type{/t}" >
       {html_options options=$MouseTypes selected=$gotoXMouseType_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoXMouseport">{t}Port{/t}</LABEL></td>
     <td>
{render acl=$gotoXMouseportACL}
      <select id="gotoXMouseport" name="gotoXMouseport" title="{t}Choose mouse port{/t}" >
       {html_options options=$MousePorts selected=$gotoXMouseport_select}
      </select>
{/render}
     </td>
    </tr>
   </table>

  </td>

  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  
  <td style="vertical-align:top;width:33%">
   <h2><img class="center" alt="" align="middle" src="plugins/systems/images/select_phone.png"> {t}Telephone hardware{/t}</h2>
   <table style="width:100%" border=0 summary="">
    <tr>
     <td>{t}Telephone{/t}&nbsp;
{render acl=$goFonHardwareACL}
	  {$hardware_list}
{/render}
     </td>
    </tr>
   </table>

  </td>
 </tr>
</table>

<table style="width:100%;" summary="">
 <tr><td colspan=5><p class="plugbottom" style="text-align:left;"></p></td></tr>
 <tr>
   <td style="width:33%;vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="plugins/goto/images/hardware.png"> {t}Graphic device{/t}</h2>
   <table summary="">
    <tr>
     <td><LABEL for="gotoXDriver">{t}Driver{/t}</LABEL></td>
     <td>
{render acl=$gotoXDriverACL}
      <select id="gotoXDriver" name="gotoXDriver" title="{t}Choose graphic driver that is needed by the installed graphic board{/t}" >
       {html_options options=$XDrivers selected=$gotoXDriver_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoXResolution">{t}Resolution{/t}</LABEL></td>
     <td>
{render acl=$gotoXResolutionACL}
      <select id="gotoXResolution" name="gotoXResolution" title="{t}Choose screen resolution used in graphic mode{/t}" >
       {html_options options=$XResolutions selected=$gotoXResolution_select}
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoXColordepth">{t}Color depth{/t}</LABEL></td>
     <td>
{render acl=$gotoXColordepthACL}
      <select id="gotoXColordepth" name="gotoXColordepth" title="{t}Choose colordepth used in graphic mode{/t}" >
       {html_options options=$XColordepths selected=$gotoXColordepth_select}
      </select>
{/render}
     </td>
    </tr>
   </table>
   </td>

  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>

   <td style="width:32%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="plugins/goto/images/display.png"> {t}Display device{/t}</h2>
   <table summary="">
    <tr>
     <td>{t}Type{/t}</td>
     <td>{$gotoXMonitor}</td>
    </tr>
    <tr>
     <td>
{render acl=$AutoSyncACL}
      <input type="checkbox" name="AutoSync" value="1" {$AutoSyncCHK} onChange="changeState('gotoXHsync');changeState('gotoXVsync');">
{/render}
     </td>
     <td>{t}Use DDC for automatic detection{/t}</td>
    </tr>
    <tr>
     <td><LABEL for="gotoXHsync">{t}HSync{/t}</LABEL></td>
     <td>
{render acl=$gotoXHsyncACL}
	<input id="gotoXHsync" name="gotoXHsync" size=10 maxlength=60 
                value="{$gotoXHsync}" title="{t}Horizontal refresh frequency for installed monitor{/t}"> kHz</td>
{/render}
    </tr>
    <tr>
     <td><LABEL for="gotoXVsync">{t}VSync{/t}</LABEL></td>
     <td>
{render acl=$gotoXVsyncACL}
      <input id="gotoXVsync"  name="gotoXVsync" size=10 maxlength=60 
                value="{$gotoXVsync}" title="{t}Vertical refresh frequency for installed monitor{/t}"> Hz</td>
{/render}
    </tr>
   </table>

   </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>

  <td style="width:33%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="plugins/systems/images/server.png"> {t}Remote desktop{/t}</h2>
   <table summary="">
    <tr>
     <td></td>
     <td><LABEL for="gotoXMethod">{t}Connect method{/t}</LABEL></td>
     <td>
{render acl=$gotoXMethodACL}
      <select name="gotoXMethod" id="gotoXMethod" title="{t}Choose method to connect to terminal server{/t}" 
			onChange="document.mainform.submit();">
       {html_options options=$XMethods selected=$gotoXMethod_select}
      </select>
	  <input type="image" src="images/lists/reload.png" alt="{t}Reload{/t}" title="{t}Reload{/t}" class="center">
{/render}
     </td>
    </tr>
    <tr>
     <td></td>
     <td colspan=2>
     <LABEL for="gotoXdmcpServer">{t}Terminal server{/t}</LABEL><br>
{render acl=$gotoXdmcpServerACL}
      <select name="selected_xdmcp_servers[]" multiple style="width:280px; height:60px;" {if $gotoXMethod_select == "default"} disabled {/if}>
		{if $gotoXMethod_select == "default"}
			{html_options values=$inherited_xdmcp_servers output=$inherited_xdmcp_servers}
		{else}
       		{html_options options=$selected_xdmcp_servers}
		{/if}
      </select><br>
{/render}
{render acl=$gotoXdmcpServerACL}
      <select name="gotoXdmcpServer_add" title="{t}Select specific terminal server to use{/t}" 
		{if $gotoXMethod_select == "default"} disabled {/if}  >
	     {html_options values=$available_xdmcp_servers output=$available_xdmcp_servers}
      </select>
{/render}
{render acl=$gotoXdmcpServerACL}
	<input type="submit" name="XdmcpAddServer" value="{msgPool type=addButton}" title="{t}Add selected server{/t}"
		{if $gotoXMethod_select == "default"} disabled {/if}>
{/render}
{render acl=$gotoXdmcpServerACL}
	<input type="submit" name="XdmcpDelServer" value="{t}Remove{/t}" title="{t}Remove selected server{/t}"
		{if $gotoXMethod_select == "default"} disabled {/if}>
{/render}

     </td>
    </tr>
   </table>
   
  </td>
 </tr>
</table>

<table style="width:100%;" summary="">
 <tr>
	<td colspan="4">
		<p class="plugbottom" style="margin-top:0px;">
		</p>
	</td>
 </tr>
 <tr>
  <td style="vertical-align:top; width:50%;">
   <h2><img class="center" alt="" align="middle" src="plugins/goto/images/scanner.png"> {t}Scan device{/t}</h2>
{render acl=$gotoScannerEnableACL}
   <input type=checkbox name="gotoScannerEnable" value="1" title="{t}Select to start SANE scan service on terminal{/t}" {$gotoScannerEnable} >
{/render}
   {t}Provide scan services{/t}

  	</td>
	<td style="border-left:1px solid #A0A0A0;">
	<td >&nbsp;
	</td>
	<td>
   		<h2>
			<img class="center" alt="" src="plugins/systems/images/select_printer.png"> {t}Printer{/t}
		</h2>
		{$gotoLpdEnable_dialog}
	</td>
 </tr>
</table>

<div style="height:40px;"></div>
