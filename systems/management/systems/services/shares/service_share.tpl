<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.goExportEntry.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.goExportEntry.subattribute}subattribute{/if}
      {if $attributes.goExportEntry.required}required{/if}
      {if !$attributes.goExportEntry.readable}nonreadable{/if}
      {if !$attributes.goExportEntry.writable}nonwritable{/if}
    ">
      <td title="{$attributes.goExportEntry.description|escape}" colspan="4">
        {eval var=$attributes.goExportEntry.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.goExportEntry.readable}nonreadable{/if}
      {if !$attributes.goExportEntry.writable}nonwritable{/if}
    ">
      <td title="{$attributes.shareName.description|escape}">
        <label for="{$attributes.shareName.htmlid}">
          {eval var=$attributes.shareName.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareName.input}</td>
      <td title="{$attributes.shareType.description|escape}">
        <label for="{$attributes.shareType.htmlid}">
          {eval var=$attributes.shareType.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareType.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.goExportEntry.readable}nonreadable{/if}
      {if !$attributes.goExportEntry.writable}nonwritable{/if}
    ">
      <td title="{$attributes.shareDesc.description|escape}">
        <label for="{$attributes.shareDesc.htmlid}">
          {eval var=$attributes.shareDesc.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareDesc.input}</td>
      <td title="{$attributes.shareCodepage.description|escape}">
        <label for="{$attributes.shareCodepage.htmlid}">
          {eval var=$attributes.shareCodepage.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareCodepage.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.goExportEntry.readable}nonreadable{/if}
      {if !$attributes.goExportEntry.writable}nonwritable{/if}
    ">
      <td title="{$attributes.sharePath.description|escape}">
        <label for="{$attributes.sharePath.htmlid}">
          {eval var=$attributes.sharePath.label}
        </label>
      </td>
      <td>{eval var=$attributes.sharePath.input}</td>
      <td title="{$attributes.shareOption.description|escape}">
        <label for="{$attributes.shareOption.htmlid}">
          {eval var=$attributes.shareOption.label}
        </label>
      </td>
      <td>{eval var=$attributes.shareOption.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.goExportEntry.readable}nonreadable{/if}
      {if !$attributes.goExportEntry.writable}nonwritable{/if}
    ">
      <td title="{$attributes.goExportEntry_buttons.description|escape}" colspan="4">
        {eval var=$attributes.goExportEntry_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
