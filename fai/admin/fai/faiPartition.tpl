
<h2>{t}Partition{/t}</h2>

<table style='width: 100%;' width="100%"> 
  <tr>
{if $FAIdiskType == "lvm"}
    <td>{t}Name{/t}</td>
    <td width="35%">
      <input type='text' name="cn" value="{$cn}">
    </td>
{else}
    <td>{t}Type{/t}</td>
    <td width="35%">
      <select name="FAIpartitionType" onChange='document.mainform.submit();'>
        {html_options options=$partitionTypes selected=$FAIpartitionType}
      </select>
      {if $cn != ""}&nbsp;({$cn}){/if}
    </td>
{/if}
    <td>{t}Description{/t}</td>
    <td width="35%">
      <input type="text" value="{$description}" name="description">
    </td>
  </tr>

{if $FAIdiskType == "disk" || $FAIdiskType == "lvm"}

  <tr>
    <td>{t}Size{/t}</td>
    <td colspan=3>
      <select name="FAIpartitionSizeType" onChange='document.mainform.submit();'>
        {html_options options=$partitionSizeTypes selected=$FAIpartitionSizeType}
      </select>
      {if $FAIpartitionSizeType != "remaining"}
        <input type='text' name='sizeStart' value='{$sizeStart}' size='5'>
        <select name='sizeStart_Type'>
          {html_options options=$sizeTypes selected=$sizeStart_Type}
        </select>
      {else}
        <input type='text' name='dummy3' value='' size='5' disabled>
        <select name='dummy4' disabled>
          {html_options options=$sizeTypes}
        </select>
      {/if}
      &nbsp;-&nbsp;
      {if $FAIpartitionSizeType != "fixed" && $FAIpartitionSizeType != "remaining"}
        <input type='text' name='sizeStop' value='{$sizeStop}' size='5'>
        <select name='sizeStop_Type'>
          {html_options options=$sizeTypes selected=$sizeStop_Type}
        </select>
      {else}
        <input type='text' name='dummy1' value='' size='5' disabled>
        <select name='dummy2' disabled>
          {html_options options=$sizeTypes}
        </select>
      {/if}
    </td>
  </tr>
  <tr>
    <td colspan="4">
      <p class="seperator">&nbsp;</p>
    </td>
  </tr>
  <tr>
    <td colspan="2">
      <input id="resize" type='checkbox' value='1' name='resize' {if $resize} checked {/if}>
      &nbsp;<label for='resize'>{t}Resize existing partition{/t}</label>
    </td>
    <td colspan="2">
      <input id="bootable" type='checkbox' value='1' name='bootable' {if $bootable} checked {/if}>
      &nbsp;<label for='bootable'>{t}Bootable{/t}</label>
    </td>
  </tr>
  <tr>
    <td colspan="4">
      <input id="preserve" type='checkbox' value='1' name='preserve' {if $preserve} checked {/if}>
      &nbsp;<label for="preserve">{t}Preserve{/t}</label>
      <select name='preserveType'>
        {html_options options=$preserveTypes selected=$preserveType}
      </select>
    </td>
  </tr>

{/if}

  <tr>
    <td colspan="4">
      <p class="seperator">&nbsp;</p>
    </td>
  </tr>

{if $FAIdiskType == "raid"}

  <tr>
    <td colspan="4">
      <h2>{t}Combined physical partitions{/t}</h2>      

      <select name='physicalPartition[]' size=5 style="width:100%; font-family: monospace;"
        multiple>
        {html_options options=$plist}
      </select>
      <br>
      <select name='physicalPartitionAdd' style='width:240px;'>
        {html_options options=$physicalPartitionList}
      </select>
      <input type="submit" name='addPhysicalPartition' value="{msgPool type="addButton"}">&nbsp; 
      <input type="submit" name='delPhysicalPartition' value="{msgPool type="delButton"}">&nbsp; 
      <input type="submit" name='toggleMissing' value="{t}Toggle missing{/t}">&nbsp; 
      <input type="submit" name='toggleSpare' value="{t}Toggle spare{/t}">&nbsp; 
    </td>
  </tr>
  <tr>
    <td colspan="4">
      <p class="seperator">&nbsp;</p>
    </td>
  </tr>
{/if}

  <tr>
    <td>
      {t}Filesystem{/t}
    </td>
    <td>
      <select name='FAIfsType' onChange="document.mainform.submit();">
        {html_options options=$FAIfsTypes selected=$FAIfsType}
      </select>
    </td>
    <td>
      {t}Filesystem create options{/t}
    </td>
    <td>
      <input type='text' name="FAIfsCreateOptions" value="{$FAIfsCreateOptions}">
    </td>
  </tr>
  <tr>
    <td colspan=2>
      <input id="encrypted" type='checkbox' name='encrypted' value='1' {if $encrypted} checked {/if}>
      &nbsp;<label for="encrypted">{t}Encrypted{/t}</label>
    </td>
    <td>
      {t}Tune options{/t}
    </td>
    <td>
      <input type='text' name="FAIfsTuneOptions" value="{$FAIfsTuneOptions}">
    </td>
  </tr>
  <tr>
    <td colspan="4">
      <p class="seperator">&nbsp;</p>
    </td>
  </tr>
  <tr>
    <td>
      {t}Mount point{/t}
    </td>
    <td>
      {if $FAIfsType != "swap"}
        <input type='text' name="FAImountPoint" value="{$FAImountPoint}">
      {else}
        <input type='text' name="dummy10" value="swap" disabled>
      {/if}
    </td>
    <td>
      {t}Mount options{/t}
    </td>
    <td>
      <input type='text' name="FAImountOptions" value="{$FAImountOptions}">
    </td>
  </tr>

  <!-- Table cell sizing  --> 
  <tr>
    <td colspan=2 style='width:50%;'></td>
    <td colspan=2 style='width:50%;'></td>
  </tr>
</table>

<input type='hidden' name='faiPartition' value='1'>
<p class='seperator'>&nbsp;</p>
<div style='text-align:right; width:100%; padding:5px'>
  {if !$freeze}
  <input type='submit' name='PartitionSave' value='{msgPool type=okButton}'>
  &nbsp;
  {/if}
  <input type='submit' name='PartitionCancel' value='{msgPool type=cancelButton}'>
</div>
