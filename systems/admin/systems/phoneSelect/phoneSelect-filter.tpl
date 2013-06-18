<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

  {if $USE_phones}
  {if $USE_mobiles}
  {$PHONE}<label for="PHONE">&nbsp;{t}Show phones{/t}</label><br/>
  {$MOBILEPHONE}<label for="MOBILEPHONE">&nbsp;{t}Show mobile phones{/t}</label><br/>
  {/if}
  {/if}

<div style="border-top:1px solid #AAAAAA"></div>

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
