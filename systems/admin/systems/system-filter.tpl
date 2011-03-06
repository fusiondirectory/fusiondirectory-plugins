<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">
  <div style="border-top:1px solid #AAAAAA"></div>

  {if $USE_goServer}
  {$SERVER}&nbsp;{t}Show servers{/t}<br>
  {/if}
  {if $USE_gotoWorkstation}
  {$WORKSTATION}&nbsp;{t}Show workstations{/t}<br>
  {/if}
  {if $USE_gotoTerminal}
  {$TERMINAL}&nbsp;{t}Show terminals{/t}<br>
  {/if}
  {if $USE_gotoPrinter}
  {$PRINTER}&nbsp;{t}Show network printer{/t}<br>
  {/if}
  {if $USE_goFonHardware}
  {$PHONE}&nbsp;{t}Show phones{/t}<br>
  {/if}
  {if $USE_sambaSamAccount}
  {$WINSTATION}&nbsp;{t}Show windows based workstations{/t}<br>
  {/if}
  {if $USE_ieee802Device}
  {$COMPONENT}&nbsp;{t}Show network devices{/t}<br>
  {/if}
  {if $USE_FAKE_OC_NewWorkstation || $USE_FAKE_OC_NewTerminal || $USE_FAKE_OC_NewServer || $USE_FAKE_OC_NewDevice || $USE_FAKE_OC_ArpNewDevice}
  {$INCOMING}&nbsp;{t}Show incoming devices{/t}<br>
  {/if}
  {if $USE_FAKE_OC_OpsiHost}
  {$OPSI}&nbsp;{t}Show OPSI based clients{/t}<br>
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
