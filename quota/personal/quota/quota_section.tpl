<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.quota.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.quota.subattribute}subattribute{/if}
      {if $attributes.quota.required}required{/if}
      {if !$attributes.quota.readable}nonreadable{/if}
      {if !$attributes.quota.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quota.description|escape}" colspan="4">
        {eval var=$attributes.quota.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quota.readable}nonreadable{/if}
      {if !$attributes.quota.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaServer.description|escape}">
        <label for="{$attributes.quotaServer.htmlid}">
          {eval var=$attributes.quotaServer.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaServer.input}</td>
      <td title="{$attributes.quotaDevice.description|escape}">
        <label for="{$attributes.quotaDevice.htmlid}">
          {eval var=$attributes.quotaDevice.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaDevice.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quota.readable}nonreadable{/if}
      {if !$attributes.quota.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaBlockSoftLimit.description|escape}">
        <label for="{$attributes.quotaBlockSoftLimit.htmlid}">
          {eval var=$attributes.quotaBlockSoftLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockSoftLimit.input}</td>
      <td title="{$attributes.quotaBlockHardLimit.description|escape}">
        <label for="{$attributes.quotaBlockHardLimit.htmlid}">
          {eval var=$attributes.quotaBlockHardLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaBlockHardLimit.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quota.readable}nonreadable{/if}
      {if !$attributes.quota.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quotaInodeSoftLimit.description|escape}">
        <label for="{$attributes.quotaInodeSoftLimit.htmlid}">
          {eval var=$attributes.quotaInodeSoftLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaInodeSoftLimit.input}</td>
      <td title="{$attributes.quotaInodeHardLimit.description|escape}">
        <label for="{$attributes.quotaInodeHardLimit.htmlid}">
          {eval var=$attributes.quotaInodeHardLimit.label}
        </label>
      </td>
      <td>{eval var=$attributes.quotaInodeHardLimit.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.quota.readable}nonreadable{/if}
      {if !$attributes.quota.writable}nonwritable{/if}
    ">
      <td title="{$attributes.quota_buttons.description|escape}" colspan="3">
        {eval var=$attributes.quota_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
