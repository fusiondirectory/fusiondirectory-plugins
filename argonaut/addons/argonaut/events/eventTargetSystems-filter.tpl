<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

  {$SERVER}&nbsp;<LABEL for='SERVER'>{t}Show servers{/t}</LABEL><br/>
  {$WORKSTATION}&nbsp;<LABEL for='WORKSTATION'>{t}Show workstations{/t}</LABEL><br/>
  {$TERMINAL}&nbsp;<LABEL for='TERMINAL'>{t}Show terminals{/t}</LABEL><br/>
  {$WINSTATION}&nbsp;<LABEL for='WINSTATION'>{t}Show winstations{/t}</LABEL><br/>
  {$BYIP}&nbsp;<LABEL for='BYIP'>{t}Filter by IP address range{/t}</LABEL><br/>

 <div style="display:block;width=100%;border-top:1px solid #AAAAAA;"></div>
 {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="geticon.php?context=actions&icon=system-search&size=16" align=middle>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
  <tr>
   <td>
    <label for="IPSTART">
     <img src="geticon.php?context=actions&icon=system-search&size=16" align=middle>&nbsp;IP Start
    </label>
   </td>
   <td>
    {$IPSTART}
   </td>
  </tr>
  <tr>
   <td>
    <label for="IPSTOP">
     <img src="geticon.php?context=actions&icon=system-search&size=16" align=middle>&nbsp;IP Stop
    </label>
   </td>
   <td>
    {$IPSTOP}
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
