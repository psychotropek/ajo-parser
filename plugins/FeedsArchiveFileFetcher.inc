<?php

/**
 * @file
 * Contains FeedsArchiveFileFetcher and FeedsArchiveFileFetcherResult.
 */

class FeedsArchiveFileFetcherResult extends FeedsFileFetcherResult {
/**
   * File list 
   *
   * @var array
   */
  public $folder_files;

}


/**
 * Fetcher files from archive files.
 */
class FeedsArchiveFileFetcher extends FeedsFileFetcher {

  /**
   * {@inheritdoc}
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $state = $source->state(FEEDS_FETCH);
    // Throws an exception if folder can not be created.
    if (!isset($state->tmp_folder)) {
      $state->tmp_folder = $this->createTemporaryExtractFolderForFile($source_config['source']);
    }

    if (!isset($state->files)) {
      // Throws an exception if archive is not valid.
      $archiver = archiver_get_archiver($source_config['source']);
      //todo: extract to public folder instead, then don't delete
      $archiver->extract($state->tmp_folder);
      $state->files = $this->listFiles($state->tmp_folder);
      $state->total = count($state->files);
    }

    if ($state->total == 0) {
      $filename = pathinfo($source_config['source'], PATHINFO_FILENAME);
      $ext = pathinfo($source_config['source'], PATHINFO_EXTENSION);
      throw new Exception(t('@filename.@ext archive file does not contain any file.', array('@filename' => $filename, '@ext' => $ext)));
    }

    $result = new FeedsFileFetcherResult($state->files);
    
    /*while ($state->files) {
      $file = array_shift($state->files);
      $state->progress($state->total, $state->total - count($state->files));
     // $result->$folder_files[] = $file['path'];
    }*/
    return $result;
    
  }

  /**
   * {@inheritdoc}
   */
  protected function listFiles($dir) {
    $mask = $this->config['filemask'] ?: '/\./';
    $files = array();
    //TODO: Test for whether this works with several files in one zip
    foreach (file_scan_directory($dir, $mask) as $file) {
      $files[] = $file->uri;
    }
    return $files;
  }

  /**
   * {@inheritdoc}
   */
  protected function getSchemes() {
    return parent::getSchemes() + array('temporary');
  }

  /**
   * {@inheritdoc}
   */
  public function configDefaults() {
    $config = parent::configDefaults();
    // Override allowed extensions.
    $config['allowed_extensions'] = archiver_get_extensions();
    // Additional configurations.
    $config['extract_directory'] = 'public://articles';
    $config['filemask'] = '/\./';
    $config['recursive'] = TRUE;
    return $config;
  }

  /**
   * {@inheritdoc}
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    $form['direct']['#title'] = t('Provide a path to an archive file');
    $form['direct']['#description'] = t('Allow users to specify a path to an archive file directly instead of uploading it to the server through the browser.');
    $form['allowed_extensions']['#description'] .= ' ' . t('Only the followed archive formats supported: @format.', array('@format' => archiver_get_extensions()));
    $form['extract_directory'] = array(
      '#type' => 'textfield',
      '#title' => t('Extract folder'),
      '#description' => t('Directory where uploaded files extracted temporary. Prefix the path with one of the available schemes: @schemes.', array('@schemes' => implode(', ', $this->getSchemes()))),
      '#default_value' => $this->config['extract_directory'],
    );
    $form['filemask'] = array(
      '#type' => 'textfield',
      '#title' => t('File mask'),
      '#description' => t('Specify a file mask to apply when scanning for files in the extract folder. See the <a href="@link" target="_blank">file_scan_directory documentation</a> for details.', array('@link' => url('http://api.drupal.org/api/function/file_scan_directory'))),
      '#default_value' => $this->config['filemask'],
    );
    $form['recursive'] = array(
      '#type' => 'checkbox',
      '#title' => t('Recursively scan extract directory'),
      '#description' => t('Recursively scan for files in extract directory.'),
      '#default_value' => $this->config['recursive'],
    );
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function configFormValidate(&$values) {
    if (!$values['extract_directory']) {
      form_set_error('extract_directory', t('Please specify an extract directory.'));
      // Do not continue validating the directory if none was specified.
      return;
    }
    // Ensure that the extract folder exists.
    elseif (!file_prepare_directory($values['extract_directory'], FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
      form_set_error('directory', t('The chosen extract folder does not exist and it can not be created.'));
    }
    parent::configFormValidate($values);
    $supported_formats = explode(' ', archiver_get_extensions());
    $selected_formats = explode(' ', $values['allowed_extensions']);
    $diff = array_diff($selected_formats, $supported_formats);
    if (!empty($diff)) {
      form_set_error('allowed_extensions', format_plural(count($diff),
          '@ext extension is not supported.',
          'Following extensions are not supported: @ext.',
          array('@ext' => count($diff) == 1 ? array_shift($diff) : implode(' ', $diff)))
      );
    }
  }

  /**
   * {@inheritdoc}
   */
  public function sourceForm($source_config) {
    $form = parent::sourceForm($source_config);
    $message = t('Allowed archive file extensions: @exts.', array('@exts' => $this->config['allowed_extensions']));
    $form['upload']['#description'] .= ' ' . $message;
    if (!empty($this->config['direct']) && !empty($form['source']['#description'])) {
      $form['source']['#description'] .= ' ' . $message;
    }
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function sourceFormValidate(&$values) {
    parent::sourceFormValidate($values);
    if ($values['source']) {
      if (is_dir($values['source'])) {
        form_set_error('feeds][FeedsFileFetcher][source', t('Directory provided instead of a file.'));
        return;
      }
      elseif (!in_array($ext = pathinfo($values['source'], PATHINFO_EXTENSION), explode(' ', $this->config['allowed_extensions']))) {
        form_set_error('feeds][FeedsFileFetcher][source', t('%ext is not a valid file extension.', array('%ext' => $ext)));
        return;
      }

      try {
        archiver_get_archiver($values['source']);
      }
      catch (Exception $e) {
        form_set_error('feeds][FeedsFileFetcher][source', t('Cannot open uploaded file, invalid archive.'));
        return;
      }
    }
  }

  /**
   * Creates a temporary folder for extracting an archive file.
   *
   * @param string $filepath
   *   Path of a archive file.
   *
   * @return string
   *   Path of the created extract folder.
   *
   * @throws Exception
   *   If folder can not be created.
   */
  protected function createTemporaryExtractFolderForFile($filepath) {
    $filename = pathinfo($filepath, PATHINFO_FILENAME);
    // Dummy fix for files with tar.gz extension. In this case the
    // returned file name is "foobar.tar".
    if (strpos($filename, '.tar', -4)) {
      $filename = substr($filename, 0, strlen($filename) - 4);
    }
    $path = $this->config['extract_directory'] . '/' . $this->id . '/' . $filename;
    $path = file_destination($path, FILE_EXISTS_RENAME);
    if (!file_prepare_directory($path, FILE_MODIFY_PERMISSIONS | FILE_CREATE_DIRECTORY)) {
      throw new Exception(sprintf('Unable to create @dir directory for extracting @file.', array('@dir' => $path, '@file' => $filename)));
    }
    return $path;
  }

}
