<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

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
  {if $USE_fdMobilePhone}
  {$MOBILEPHONE}<label for="MOBILEPHONE">&nbsp;{t}Show mobile phones{/t}</label><br>
  {/if}

  <div style="display:block;width=100%;border-top:1px solid #AAAAAA;"></div>

 {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
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
 </table>

 <table  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td style="width:100%;text-align:right;">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
