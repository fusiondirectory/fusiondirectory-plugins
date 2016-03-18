<br>
<b>{t}Warning{/t}:</b> {t}Actions you choose here influence all systems in this object group. Additionally, all values editable here can be inherited by the clients assigned to this object group.{/t}
<br>
<br>
<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>
<h2><img class="center" alt="" align="middle" src="geticon.php?context=devices&icon=terminal&size=16"> {t}Generic{/t}</h2>

<table width="100%">
  <tr>
    <td style="vertical-align:top;border-left:1px solid #A0A0A0;">
    <!-- Upper right -->

      <table>
        <tr>
         <td><label for="gotoMode">{t}Mode{/t}</label></td>
         <td>
{render acl=$gotoModeACL}
          <select name="gotoMode" id="gotoMode" title="{t}Select terminal mode{/t}">
           {html_options options=$modes selected=$gotoMode_select}
          </select>
{/render}
         </td>
        </tr>
        <tr><td colspan="2">&nbsp;</td></tr>
        {if $is_termgroup}
            <tr>
             <td><LABEL for="gotoTerminalPath">{t}Root server{/t}</LABEL></td>
             <td>
        {render acl=$gotoTerminalPathACL}
              <select name="gotoTerminalPath" id="gotoTerminalPath" title="{t}Select NFS root filesystem to use{/t}">
               {html_options options=$nfsservers selected=$gotoTerminalPath_select}
              </select>
        {/render}
             </td>
            </tr>
            <tr>
             <td><LABEL for="gotoSwapServer">{t}Swap server{/t}</LABEL></td>
             <td>
        {render acl=$gotoSwapServerACL}
              <select name="gotoSwapServer" id="gotoSwapServer" title="{t}Choose NFS filesystem to place swap files on{/t}">
               {html_options options=$swapservers selected=$gotoSwapServer_select}
              </select>
        {/render}
             </td>
            </tr>
        {/if}
           </table>
    </td>
  </tr>
  <tr>
    <td colspan=2>
      <p class="seperator">&nbsp;</p>
    </td>
  </tr>
  <tr>
    <td>
    <!-- Bottom left -->

    </td>
    <td style="vertical-align:top;border-left:1px solid #A0A0A0;">
    <!-- Bottom right -->

    </td>
  </tr>
</table>
<input name="workgeneric_posted" value="1" type="hidden">


<h2><img class="center" alt="" align="middle" src="geticon.php?context=types&icon=action&size=16"> {t}Action{/t}</h2>
        <table>
         <tr>
          <td>
{render acl=$FAIstateACL}
           <select size="1" name="saction" {$FAIstateACL} title="{t}Select action to execute for this terminal{/t}">
          <option>&nbsp;</option>
          {html_options options=$actions}
           </select>
{/render}
          </td>
          <td>
{render acl=$FAIstateACL}
           <input type=submit name="action" value="{t}Execute{/t}">
{/render}
          </td>
         </tr>
        </table>





