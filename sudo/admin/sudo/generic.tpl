{if $is_default}

<h2>{t}Generic{/t} - {t}global defaults{/t}</h2>
<table style="width:100%;">
 <tr>
  <td>
   {t}Name{/t}{$must}
  </td>
  <td>
   <input type="text" name="dummy" value="{$cn}" disabled>
  </td>
 </tr>
 <tr>
  <td>
   {t}Description{/t}
  </td>
  <td>
   {render acl=$descriptionACL}
   <input type="text" name="description" value="{$description}">
   {/render}
  </td>
 </tr>
</table>

{else}

<table style="width:100%;">
 <tr>
  <td style="vertical-align:top;width:50%">
   <h2>{t}Generic{/t}</h2>
   <table> 
    <tr>
     <td>
      {t}Name{/t}{$must}
     </td>
     <td>
      {render acl=$cnACL}
      <input type="text" name="cn" value="{$cn}">
      {/render}
     </td>
    </tr>
    <tr>
     <td>
      {t}Description{/t}
     </td>
     <td>
      {render acl=$descriptionACL}
      <input type="text" name="description" value="{$description}">
      {/render}
     </td>
    </tr>
   </table>
  </td>
  <td style="padding-left:5px;border-left: solid 1px #AAAAAA;">
   <h2><img alt="" class="center" align="middle" src="images/lists/locked.png" />&nbsp; {t}System trust{/t}</h2>
    {t}Trust mode{/t}&nbsp; 
    {render acl=$trustModelACL}
        <select name="trustmode" id="trustmode" size=1
            onChange="changeSelectState('trustmode', 'wslist');
                      changeSelectState('trustmode', 'add_ws');
                      changeSelectState('trustmode', 'del_ws');">
          {html_options options=$trustmodes selected=$trustmode}
        </select>
		<br>
    {/render}
    {render acl=$trustModelACL}
       <select style="width:100%" id="wslist" name="workstation_list[]" size=5 multiple {$trusthide}>
        {html_options values=$workstations output=$workstations}
        {if $emptyArrAccess}
            <option disabled>&nbsp;</option>
        {/if}
       </select>
    {/render}
       <br>
    {render acl=$trustModelACL}
       <input type="submit" id="add_ws" value="{msgPool type=addButton}" name="add_ws" {$trusthide}>&nbsp;
    {/render}
    {render acl=$trustModelACL}
       <input type="submit" id="del_ws" value="{msgPool type=delButton}" name="delete_ws" {$trusthide}>
    {/render}

  </td>
 </tr> 
 <tr><td style="width:100%;"colspan="2"><p class="seperator">&nbsp;</p></td></tr>
 <tr>
  <td style="width:50%;padding-right:5px;">
   <h2><img src="plugins/users/images/select_user.png" alt="" class="center">&nbsp;{t}Users and groups{/t}</h2>
   {render acl=$sudoUserACL}
   {$divlist_sudoUser}
   {/render}
   {render acl=$sudoUserACL}
   <input type='text' value='' name='new_sudoUser'>
   {/render}
   {render acl=$sudoUserACL}
   <input type='submit' name='add_sudoUser' value='{msgPool type=addButton}'>
   {/render}
   {render acl=$sudoUserACL}
   <input type='submit' name='list_sudoUser' value='{t}Add from list{/t}'>
   {/render}
  </td>
  <td style="padding-left:5px;border-left: solid 1px #AAAAAA;">
   <h2><img src="plugins/sudo/images/select_workstation.png" alt="" class="center">&nbsp;{t}Systems{/t}</h2>
   {render acl=$sudoHostACL}
   {$divlist_sudoHost}
   {/render}
   {render acl=$sudoHostACL}
   <input type='text' value='' name='new_sudoHost'>
   {/render}
   {render acl=$sudoHostACL}
   <input type='submit' name='add_sudoHost' value='{msgPool type=addButton}'>
   {/render}
   {render acl=$sudoHostACL}
   <input type='submit' name='list_sudoHost' value='{t}Add from list{/t}'>
   {/render}
  </td>
 </tr> 
 <tr><td style="width:100%;"colspan="2"><p class="seperator">&nbsp;</p></td></tr>
 <tr>
  <td style="padding-right:5px;">
   <h2><img src="images/rocket.png" alt="" class="center">&nbsp;{t}Commands{/t}</h2>
   {render acl=$sudoCommandACL}
   {$divlist_sudoCommand}
   {/render}
   {render acl=$sudoCommandACL}
   <input type='text' value='' name='new_sudoCommand'>
   {/render}
   {render acl=$sudoCommandACL}
   <input type='submit' name='add_sudoCommand' value='{msgPool type=addButton}'>
   {/render}
  </td>
  <td style="padding-left:5px;border-left: solid 1px #AAAAAA;">
   <h2><img src="plugins/users/images/select_user.png" alt="" class="center">&nbsp;{t}Run as{/t}</h2>
   {render acl=$sudoRunAsACL}
   {$divlist_sudoRunAs}
   {/render}
   {render acl=$sudoRunAsACL}
   <input type='text' value='' name='new_sudoRunAs'>
   {/render}
   {render acl=$sudoRunAsACL}
   <input type='submit' name='add_sudoRunAs' value='{msgPool type=addButton}'>
   {/render}
  </td>
 </tr>
</table>
{/if}
