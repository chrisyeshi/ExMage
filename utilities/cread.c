#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

int readData(const char* filename,
              int region_index[3], int region_count[3],
              char attributes[][50], int attribute_count,
              float* flow_field[])
{
  herr_t status;
  hid_t file_id;
  hid_t dset_id[attribute_count];
  // open file spaces
  hid_t acc_tpl = H5Pcreate(H5P_FILE_ACCESS);
  status = H5Pset_fapl_mpio(acc_tpl, MPI_COMM_WORLD, MPI_INFO_NULL);
  file_id = H5Fopen(filename, H5F_ACC_RDONLY, acc_tpl);
  if (file_id < 0)
    return 0;
  status = H5Pclose(acc_tpl);

  int i = 0;
  for (i = 0; i < attribute_count; ++i)
  {
    dset_id[i] = H5Dopen(file_id, attributes[i], H5P_DEFAULT);

    hid_t spac_id = H5Dget_space(dset_id[i]);
    hsize_t htotal_size3[3];
    status = H5Sget_simple_extent_dims(spac_id, htotal_size3, NULL);
    hsize_t region_size3[3] = {htotal_size3[0] / region_count[0],
                               htotal_size3[1] / region_count[1],
                               htotal_size3[2] / region_count[2]};
    hsize_t start[3] = {region_index[0] * region_size3[0],
                        region_index[1] * region_size3[1],
                        region_index[2] * region_size3[2]};
    hsize_t count[3] = {region_size3[0], region_size3[1], region_size3[2]};
    status = H5Sselect_hyperslab(spac_id, H5S_SELECT_SET, start, NULL, count, NULL);
    hid_t memspace = H5Screate_simple(3, count, NULL);

    flow_field[i] = (float *) malloc(count[0] * count[1] * count[2] * sizeof(float));
    status = H5Dread(dset_id[i], H5T_NATIVE_FLOAT, memspace, spac_id, H5P_DEFAULT, flow_field[i]);

    H5Dclose(dset_id[i]);
  }
  H5Fclose(file_id);
  return 1;
}
