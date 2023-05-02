<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.quotaDeviceParameters.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.quotaDeviceParameters.subattribute}subattribute{/if}
      {if $attributes.quotaDeviceParameters.required}required{/if}
      {if !$attributes.quotaDeviceParameters.readable}nonreadable{/if}
      {if !$attributes.quotaDeviceParameters.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaDeviceParameters.description|escape}" colspan="4">
        {eval var=$attributes.quotaDeviceParameters.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quotaDeviceParameters.readable}nonreadable{/if}
      {if !$attributes.quotaDeviceParameters.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaDevice.description|escape}">
        <label for="{$attributes.quotaDevice.htmlid}">
          {eval var=$attributes.quotaDevice.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaDevice.input}</td>
      <td title="{$attributes.quotaBlockSize.description|escape}">
        <label for="{$attributes.quotaBlockSize.htmlid}">
          {eval var=$attributes.quotaBlockSize.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockSize.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quotaDeviceParameters.readable}nonreadable{/if}
      {if !$attributes.quotaDeviceParameters.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaDesc.description|escape}">
        <label for="{$attributes.quotaDesc.htmlid}">
          {eval var=$attributes.quotaDesc.label}
        </label>
      </td>
      <td colspan="3">{eval var=$attributes.quotaDesc.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quotaDeviceParameters.readable}nonreadable{/if}
      {if !$attributes.quotaDeviceParameters.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaDeviceParameters_buttons.description|escape}" colspan="4">
        {eval var=$attributes.quotaDeviceParameters_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
