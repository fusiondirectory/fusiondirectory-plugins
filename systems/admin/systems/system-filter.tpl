<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">
  <div style="border-top:1px solid #AAAAAA"></div>

  {if 0}
  <input class="filter_checkbox" id="filter_check_all" name="filter_check_all" onclick="chk_set_all_by_class('filter_checkbox',document.getElementById('filter_check_all').checked);document.mainform.submit();" type="checkbox">
  <hr/>
  {/if}

  {if $USE_goServer}
  {$SERVER}<label for="SERVER">&nbsp;{t}Show servers{/t}</label><br>
  {/if}
  {if $USE_gotoWorkstation}
  {$WORKSTATION}<label for="WORKSTATION">&nbsp;{t}Show workstations{/t}</label><br>
  {/if}
  {if $USE_gotoTerminal}
  {$TERMINAL}<label for="TERMINAL">&nbsp;{t}Show terminals{/t}</label><br>
  {/if}
  {if $USE_gotoPrinter}
  {$PRINTER}<label for="PRINTER">&nbsp;{t}Show network printer{/t}</label><br>
  {/if}
  {if $USE_goFonHardware}
  {$PHONE}<label for="PHONE">&nbsp;{t}Show phones{/t}</label><br>
  {/if}
  {if $USE_sambaSamAccount}
  {$WINSTATION}<label for="WINSTATION">&nbsp;{t}Show windows based workstations{/t}</label><br>
  {/if}
  {if $USE_ieee802Device}
  {$COMPONENT}<label for="COMPONENT">&nbsp;{t}Show network devices{/t}</label><br>
  {/if}
  {if $USE_FAKE_OC_NewWorkstation || $USE_FAKE_OC_NewTerminal || $USE_FAKE_OC_NewServer || $USE_FAKE_OC_NewDevice || $USE_FAKE_OC_ArpNewDevice}
  {$INCOMING}<label for="INCOMING">&nbsp;{t}Show incoming devices{/t}</label><br>
  {/if}

  <div style="border-top:1px solid #AAAAAA"></div>

 {$SCOPE}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="images/lists/search.png" align=middle>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
  <tr>
   <td>
    <label for="NAME">
     <img src="plugins/users/images/select_user.png" align=middle>&nbsp;User
    </label>
   </td>
   <td>
    {$USER}
   </td>
  </tr>
 </table>

 <table summary=""  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td width="100%" align="right">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
